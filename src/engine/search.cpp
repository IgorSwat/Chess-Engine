#include "transpositionTable.h"
#include "../logic/pieces.h"
#include "../logic/randomGens.h"
#include <numeric>


namespace Search {

    // -----------------------------
    // Crawler methods - main search
    // -----------------------------

    Value Crawler::search(Depth depth)
    {
        lastUsedDepth = depth;

        non_leaf_nodes = leaf_nodes = qs_nodes = 0;

        if (depth == 0)
            return evaluate();

        Value score = search<ROOT_NODE, false>(-MAX_EVAL, MAX_EVAL, std::min(depth, static_cast<Depth>(MAX_SEARCH_DEPTH)));
        ssTop = searchStack;

        return score;
    }

    template <NodeType node, bool nmpAvailable>
    Value Crawler::search(Value alpha, Value beta, Depth depth)
    {
        pushStack();

        // Testing block
        if (depth != 0)
            non_leaf_nodes++;
        else
            leaf_nodes++;

        // 50-move rule
        if (virtualBoard.halfmovesClocked() >= 100)
            return 0;
        
        // Repetitions
        if (virtualBoard.irreversibleMoveDistance() >= 4) {
            std::uint16_t repetitions = virtualBoard.countRepetitions();

            if (repetitions == 3)   // A definite 3-fold
                return 0;
            if (repetitions == 2 && lastUsedDepth - depth > virtualBoard.irreversibleMoveDistance())    // A repetition inside search space
                return 0;
        }

        // Check the transposition table
        Move ttMove;
        const TranspositionTable::Entry* entry = tTable->probe(virtualBoard.hash(), virtualBoard.pieces());
        if (entry) {
            if constexpr (SEARCH_MODE == TRACE && node == ROOT_NODE) {
                std::cout << "[TestEngine]: found existing entry in T.Table: [score: " << relative_score(entry->score, &virtualBoard);
                std::cout << ", type: " << int(entry->typeOfNode) << ", best " << entry->bestMove << "]\n";
            }

            if (entry->depth >= depth &&
                (entry->typeOfNode & PV_NODE ||
                 entry->typeOfNode == CUT_NODE && entry->score >= beta ||
                 entry->typeOfNode == ALL_NODE && entry->score <= alpha))
                return entry->score;
                
            ttMove = entry->bestMove;
            if (depth > 0 && ttMove != Move::null()) {
                virtualBoard.makeMove(ttMove);
                Value score = -search<PV_NODE, true>(-beta, -alpha, depth - 1);
                virtualBoard.undoLastMove();
                ssTop--;

                if (score >= beta) {
                    tTable->set({
                        virtualBoard.hash(),
                        virtualBoard.pieces(),
                        depth,
                        score,  // score
                        ttMove,
                        CUT_NODE,
                        virtualBoard.halfmovesPlain()
                    }, rootAge);

                    return score;
                }

                if (score > ssTop->score) {
                    ssTop->score = score;
                    ssTop->bestMove = ttMove;
                    if (score > alpha) {
                        alpha = score;
                        ssTop->node = PV_NODE;
                    }
                }
            }

            ssTop->eval = entry->score;
        }

        // Go to quiescence if maximum depth reached
        if (depth <= 0)
            return quiescence<ROOT_NODE>(alpha, beta, MAX_QUIESCENCE_DEPTH);
        
        ssTop->staticEval = evaluate();

        // Null move pruning heuristic
        // Note: Since it is used before stealmate condition check, it might make some bugs in extremaly rare positions.
        //       Luckily it does not affect checkmate positions, since it cannot be used when side on move is in check.
        if constexpr (false && nmpAvailable) {
            // Null move pruning failes in zugzwang, so we want to avoid using it in late endgames, where zugzwang positions are most common.
            // Also, we want to prevent it from running twice in a row in the same branch.
            if (!virtualBoard.isInCheck() && virtualBoard.gameStage() > Evaluation::SINGLE_ROOK_VS_ROOK_ENDGAME) {
                int threats = evaluator.getThreatCount(virtualBoard.movingSide());
                Value betterEval = ssTop->eval != NO_EVAL ? ssTop->eval : ssTop->staticEval;

                if (threats == 0 && (depth < NPM_CHECK_MIN_DEPTH || betterEval - NPM_ACTIVATION_THRESHOLD > beta)) {
                    virtualBoard.makeNullMove();

                    // NPM is not available at root node, so nextStage has to be COMMON_STAGE
                    // nmpAvailable = false to prevent double null move in two consecutive nodes
                    Value score = -search<NON_PV_NODE, false>(-beta, -alpha, depth - 1);
                    ssTop--;

                    // If score already exceeds beta (with side on move playing one move less), then beta-cutoff is almost sure
                    if (score >= beta) {
                        virtualBoard.undoNullMove();
                        tTable->set({
                            virtualBoard.hash(),
                            virtualBoard.pieces(),
                            depth,
                            score,  // score
                            Move::null(),
                            CUT_NODE,
                            virtualBoard.halfmovesPlain()
                        }, rootAge);

                        return score;
                    }

                    virtualBoard.undoNullMove();
                }
            }
        }

        // Initial move generation
        MoveSelector moveSelector(&virtualBoard);
        if (ssTop->ply < 2) {
            moveSelector.generateMoves<MoveGeneration::LEGAL>();

            // Sort moves by static evaluation of the following position
            MoveSelection::sort_moves(moveSelector, [this](const Move& move) -> int64_t {
                this->virtualBoard.makeMove(move);
                Value eval = -this->evaluate();
                this->virtualBoard.undoLastMove();

                // This makes SEE a primary sorting parameter
                // Static evaluation is considered only when SEE of two moves is equal
                return 3LL * MAX_EVAL * SEE::evaluate(&this->virtualBoard, move) + eval;
            });
        }
        else {
            if (virtualBoard.isInCheck(virtualBoard.movingSide()))
                moveSelector.generateMoves<MoveGeneration::CHECK_EVASION>();
            else
                moveSelector.generateMoves<MoveGeneration::CAPTURE>();
        }

        // Detect mate, stealmate and other types of draw
        if (!moveSelector.hasMoves()) {
            Value score = virtualBoard.isInCheck() ? -MAX_EVAL : 0;      // Mate or stealmate
            tTable->set({
                virtualBoard.hash(),
                virtualBoard.pieces(),
                depth,
                score,
                Move::null(),
                TERMINAL_NODE,
                virtualBoard.halfmovesPlain()
            }, rootAge);

            // The score is exact, so we can finish searching the branch here
            return score;
        }

        if constexpr (SEARCH_MODE == TRACE && node == ROOT_NODE)
            std::cout << "Analyzing the following moves:\n";

        // Specify selection strategy according to search stage
        GenerationStrategy genStrategy = ssTop->ply >= 2 ? GenerationStrategy::CASCADE : GenerationStrategy::STRICT;
        SelectionStrategy selStrategy = ssTop->ply >= 2 ? SelectionStrategy::STANDARD_ORDERING : SelectionStrategy::SIMPLE;
        
        // Main loop
        Move move = moveSelector.selectNext(genStrategy, selStrategy);
        while (move != Move::null()) {
            // Avoid repeating of transposition table suggestion
            if (move == ttMove) {
                move = moveSelector.selectNext(genStrategy, selStrategy);
                continue;
            }

            // Futility pruning - discard quiet moves with no perspective of raising alpha
            if (depth == 1 && !virtualBoard.isInCheck() &&
                !move.isCapture() && !move.isPromotion() && !virtualBoard.isCheck(move) &&
                ssTop->staticEval + FUTILITY_MARGIN < alpha) {

                move = moveSelector.selectNext(genStrategy, selStrategy);
                continue;
            }

            // Make move and evaluate further
            virtualBoard.makeMove(move);
            Value score = -search<PV_NODE, true>(-beta, -alpha, depth - 1);
            virtualBoard.undoLastMove();
            ssTop--;

            if constexpr (SEARCH_MODE == TRACE && node == ROOT_NODE)
                std::cout << move << std::dec << " -> [score: " << relative_score(score, &virtualBoard) << "]\n";

            // Beta cut-off: an enemy would never allow a line worse than beta to occur
            if (score >= beta) {
                tTable->set({
                    virtualBoard.hash(),
                    virtualBoard.pieces(),
                    depth,
                    score,  // score
                    move,
                    CUT_NODE,
                    virtualBoard.halfmovesPlain()
                }, rootAge);
                return score;
            }

            // PV node - a move that is better for moving side than previous best one
            if (score > ssTop->score) {
                ssTop->score = score;
                ssTop->bestMove = move;
                if (score > alpha) {
                    alpha = score;
                    ssTop->node = PV_NODE;
                }
            }

            move = moveSelector.selectNext(genStrategy, selStrategy);
        }

        // Save search results in transposition table
        tTable->set({
            virtualBoard.hash(),
            virtualBoard.pieces(),
            depth,
            ssTop->score,
            ssTop->bestMove,
            ssTop->node,
            virtualBoard.halfmovesPlain()
        }, rootAge);

        return ssTop->score;
    }


    // -----------------------------------
    // Crawler methods - quiescence search
    // -----------------------------------

    template <NodeType node>
    Value Crawler::quiescence(Value alpha, Value beta, Depth depth)
    {
        qs_nodes++;

        if constexpr (node != ROOT_NODE)
            pushStack();
        
        if constexpr (node != ROOT_NODE) {
            const TranspositionTable::Entry* entry = tTable->probe(virtualBoard.hash(), virtualBoard.pieces());
            if (entry &&
                (entry->typeOfNode & PV_NODE ||
                 entry->typeOfNode == CUT_NODE && entry->score >= beta ||
                 entry->typeOfNode == ALL_NODE && entry->score <= alpha))
                return entry->score;
        }

        MoveSelector moveSelector(&virtualBoard);
        if (virtualBoard.isInCheck())
            moveSelector.generateMoves<MoveGeneration::CHECK_EVASION>();
        else
            moveSelector.generateMoves<MoveGeneration::CAPTURE>();

        // Detect mate, stealmate and other types of draw
        if (!moveSelector.hasMoves()) {
            Value score = virtualBoard.isInCheck() ? -MAX_EVAL : 0;      // Mate or stealmate
            tTable->set({
                virtualBoard.hash(),
                virtualBoard.pieces(),
                depth,
                score,
                Move::null(),
                TERMINAL_NODE,
                virtualBoard.halfmovesPlain()
            }, rootAge);

            // The score is exact, so we can finish searching the branch here
            return score;
        }

        // Stand pat score - a static evaluation to help asses whether some moves are worth trying out or not
        ssTop->staticEval = evaluate();

        // Maximum queiescence depth reached
        if (depth == 0)
            return ssTop->staticEval;

        Move move;

        // Check all "good" captures which could boost the score near the alpha (upper bound) region
        if (moveSelector.currGenType >= MoveGeneration::CAPTURE) {
            // Experimental - sort moves by SEE at initial quiescence depth
            if constexpr (node == ROOT_NODE)
                moveSelector.sortCaptures();

            constexpr SelectionStrategy selStrategy = node == ROOT_NODE ? SelectionStrategy::SIMPLE : SelectionStrategy::STANDARD_ORDERING;

            move = moveSelector.selectNext(GenerationStrategy::STRICT, selStrategy);
            while (move.see > 0) {
                // Delta pruning
                if (ssTop->staticEval + move.see + DELTA_MARGIN < alpha && moveSelector.currGenType != MoveGeneration::CHECK_EVASION) {
                    if constexpr (node == ROOT_NODE)
                        break;
                    else {
                        move = moveSelector.selectNext(GenerationStrategy::STRICT, selStrategy);
                        continue;
                    }
                }

                virtualBoard.makeMove(move);
                Value score = -quiescence<NON_PV_NODE>(-beta, -alpha, depth - 1);
                virtualBoard.undoLastMove();
                ssTop--;

                if (score >= beta)
                    return score;
                
                if (score > ssTop->score) {
                    ssTop->score = score;
                    if (score > alpha)
                        alpha = score;
                }
                
                move = moveSelector.selectNext(GenerationStrategy::STRICT, selStrategy);
            }
        }

        int noThreats = evaluator.getThreatCount(virtualBoard.movingSide());
        Bitboard threats = evaluator.getThreatMap(virtualBoard.movingSide());

        // Consider moving threatened pieces or capture attacking pieces to stabilize
        if (ssTop->staticEval + EPSILON_MARGIN > alpha && (noThreats > 1 || virtualBoard.isInCheck())) {
            if (move == Move::null())
                move = moveSelector.selectNext(GenerationStrategy::CASCADE, SelectionStrategy::SIMPLE);
            while (move != Move::null()) {
                PieceType pType = type_of(virtualBoard.onSquare(move.to()));
                Bitboard attacks = pType == NULL_TYPE ? 0 :
                                   pType == PAWN ? Pieces::pawn_attacks(~virtualBoard.movingSide(), move.to()) :
                                                   Pieces::piece_attacks_d(pType, move.to(), virtualBoard.pieces());
                if (virtualBoard.isInCheck() ||
                    moveSelector.currGenType != MoveGeneration::CAPTURE && threats & move.from() ||
                    moveSelector.currGenType == MoveGeneration::CAPTURE &&
                    ssTop->staticEval + EPSILON_MARGIN + move.see > alpha &&
                    (threats & move.from() || attacks & threats)
                ) {
                    virtualBoard.makeMove(move);
                    Value score = -quiescence<NON_PV_NODE>(-beta, -alpha, depth - 1);
                    virtualBoard.undoLastMove();
                    ssTop--;

                    if (score >= beta)
                        return score;
                    
                    if (score > ssTop->score) {
                        ssTop->score = score;
                        if (score > alpha)
                            alpha = score;
                    }
                }

                move = moveSelector.selectNext(GenerationStrategy::CASCADE, SelectionStrategy::SIMPLE);
            }

            return ssTop->score;
        }
        
        return std::max(ssTop->staticEval, ssTop->score);
    }

}