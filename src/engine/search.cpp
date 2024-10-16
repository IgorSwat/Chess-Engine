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
        // -------------

        if (depth != 0)
            non_leaf_nodes++;
        else
            leaf_nodes++;

        // Stage 1 - detect draws related to game rules
        // --------------------------------------------
        // 1. 50-move rule
        // 2. Repetitions

        // 50-move rule
        if (virtualBoard.halfmovesClocked() >= 100)
            return 0;
        
        // Repetitions
        if (virtualBoard.irreversibleMoveDistance() >= 4) {
            std::uint16_t repetitions = virtualBoard.countRepetitions();

            if (repetitions == 3)   // A definite 3-fold
                return 0;
            if (repetitions == 2 && ssTop->ply > virtualBoard.irreversibleMoveDistance())    // A repetition inside search space
                return 0;
        }

        // Stage 2 - transposition table probe
        // -----------------------------------
        // 1. If score is exact (pv or terminal node), or if score holds in other types of nodes, perform a cut-off
        // 2. Otherwise try transposition table best / refutation move

        Move ttMove;
        const TranspositionTable::Entry* entry = tTable->probe(virtualBoard.hash(), virtualBoard.pieces());
        if (entry) {
            if constexpr (SEARCH_MODE == TRACE && node == ROOT_NODE) {
                std::cout << "[TestEngine]: found existing entry in T.Table: [score: " << std::dec << relative_score(entry->score, &virtualBoard);
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

        // Stage 2.5 - quiescence search
        // -----------------------------
        // 1. Go to quiescence if maximum depth reached
        // 2. Do not go further into search routine

        if (depth <= 0)
            return quiescence<ROOT_NODE>(alpha, beta, MAX_QUIESCENCE_DEPTH);
        
        // Stage 3 - static eval
        // ---------------------
        // 1. Statically evaluate current position
        // 2. Use obtained knowledge to improve further move ordering and apply various heuristics (Futility Pruning, etc.)
        
        ssTop->staticEval = evaluate();

        // Stage 4 - NMP (Null Move Pruning) heuristic
        // -------------------------------------------
        // 1. Check the conditions for NMP
        // 2. If conditions are met and cut-node is likely to happen in currect ply, perform search at lower depth
        // 3. If obtained score exceeds beta, perform a cut-off

        // Note: Since it is used before stealmate condition check, it might make some bugs in extremaly rare positions.
        //       Luckily it does not affect checkmate positions, since it cannot be used when side on move is in check.
        if constexpr (false && nmpAvailable) {
            // Null move pruning failes in zugzwang, so we want to avoid using it in late endgames, where zugzwang positions are most common.
            if (!virtualBoard.isInCheck() && virtualBoard.gameStage() > Evaluation::SINGLE_ROOK_VS_ROOK_ENDGAME) {
                int threats = evaluator.threatCount[virtualBoard.movingSide()];
                Value betterEval = ssTop->eval != NO_EVAL ? ssTop->eval : ssTop->staticEval;

                if (depth > 1 && threats == 0 && betterEval - NPM_ACTIVATION_THRESHOLD > beta) {
                    virtualBoard.makeNullMove();

                    Value score = -search<NON_PV_NODE, false>(-beta, -beta + 1, depth - 1);
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

        // Stage 5 - Initial move generation
        // ---------------------------------
        // 1. Generate moves according to current position, depth and strategy

        MoveSelector moveSelector(&virtualBoard, &evaluator);
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
            moveSelector.strategy = MoveSelection::STANDARD_ORDERING;
        }

        // Stage 6 - checkmate & stealmate detection
        // -----------------------------------------
        // 1. Detect checkmate and stealmate which result in terminal node

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

        // Stage 7 - move ordering strategies
        // ----------------------------------

        bool cascadeSelection = ssTop->ply >= 2;
        // Prioritize aggresive moves, except for late endgames
        if (virtualBoard.gameStage() > 80)
            moveSelector.strategy |= MoveSelection::make_strategy(MoveSelection::THREAT_CREATION, MoveGeneration::QUIET_CHECK) |
                                     MoveSelection::make_strategy(MoveSelection::THREAT_CREATION, MoveGeneration::QUIET);
        if (evaluator.threatCount[virtualBoard.movingSide()] > 0)
            moveSelector.strategy |= MoveSelection::make_strategy(MoveSelection::THREAT_EVASION, MoveGeneration::QUIET_CHECK) |
                                     MoveSelection::make_strategy(MoveSelection::THREAT_EVASION, MoveGeneration::QUIET);
        
        // Stage 8 - main search loop
        // --------------------------

        Move move = moveSelector.selectNext(cascadeSelection);
        while (move != Move::null()) {
            // Avoid repeating of transposition table suggestion
            if (move == ttMove) {
                move = moveSelector.selectNext(cascadeSelection);
                continue;
            }

            // Stage 9 - futility pruning
            // --------------------------
            // 1. Discard quiet moves with no perspective of raising alpha

            if (depth == 1 && !virtualBoard.isInCheck() &&
                !move.isCapture() && !move.isPromotion() && !virtualBoard.isCheck(move) &&
                ssTop->staticEval + FUTILITY_MARGIN < alpha) {

                move = moveSelector.selectNext(cascadeSelection);
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

            move = moveSelector.selectNext(cascadeSelection);
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

        MoveSelector moveSelector(&virtualBoard, &evaluator);
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
            else
                moveSelector.strategy = MoveSelection::STANDARD_ORDERING;

            move = moveSelector.selectNext(false);
            while (SEE::evaluate(&virtualBoard, move) > 0) {
                // Delta pruning
                if (ssTop->staticEval + move.see + DELTA_MARGIN < alpha && moveSelector.currGenType != MoveGeneration::CHECK_EVASION) {
                    if constexpr (node == ROOT_NODE)
                        break;
                    else {
                        move = moveSelector.selectNext(false);
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
                
                move = moveSelector.selectNext(false);
            }
        }

        int noThreats = evaluator.threatCount[virtualBoard.movingSide()];
        Bitboard threats = evaluator.threatMap[virtualBoard.movingSide()];

        // Consider moving threatened pieces or capture attacking pieces to stabilize
        if (ssTop->staticEval + EPSILON_MARGIN > alpha && (noThreats > 1 || virtualBoard.isInCheck())) {
            if constexpr (node != ROOT_NODE)
                moveSelector.strategy = MoveSelection::SIMPLE_ORDERING;

            if (move == Move::null())
                move = moveSelector.selectNext(true);
            while (move != Move::null()) {
                PieceType pType = type_of(virtualBoard.onSquare(move.to()));
                Bitboard attacks = pType == NULL_TYPE ? 0 :
                                   pType == PAWN ? Pieces::pawn_attacks(~virtualBoard.movingSide(), move.to()) :
                                                   Pieces::piece_attacks_d(pType, move.to(), virtualBoard.pieces());
                if (virtualBoard.isInCheck() ||
                    moveSelector.currGenType != MoveGeneration::CAPTURE && threats & move.from() ||
                    moveSelector.currGenType == MoveGeneration::CAPTURE &&
                    ssTop->staticEval + EPSILON_MARGIN + SEE::evaluate(&virtualBoard, move) > alpha &&
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

                move = moveSelector.selectNext(true);
            }

            return ssTop->score;
        }
        
        return std::max(ssTop->staticEval, ssTop->score);
    }

}