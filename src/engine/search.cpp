#include "searchConfig.h"
#include "transpositionTable.h"
#include "../logic/pieces.h"
#include "../logic/randomGens.h"
#include <numeric>


namespace Search {

    // --------------
    // Helper defines
    // --------------

    constexpr Value MAX_EVAL = std::numeric_limits<Value>::max();   // Used as a checkmate evaluation and upper boundary for beta
    constexpr Value NO_EVAL = MAX_EVAL - 1;


    // -----------------------------
    // Crawler methods - main search
    // -----------------------------

    Value Crawler::search(Depth depth)
    {
        lastUsedDepth = depth;

        non_leaf_nodes = leaf_nodes = qs_nodes = 0;

        if (depth == 0)
            return evaluate();

        return search<ROOT_STAGE_I, false>(-MAX_EVAL, MAX_EVAL, depth);
    }

    template <SearchStage stage, bool nmpAvailable>
    Value Crawler::search(Value alpha, Value beta, Depth depth)
    {
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

        Value static_eval = NO_EVAL, eval = NO_EVAL;

        // Check the transposition table
        Move suggestedMove;
        const TranspositionTable::Entry* entry = tTable->probe(virtualBoard.hash(), virtualBoard.pieces());
        if (entry) {
            if constexpr (SEARCH_MODE == TRACE && stage == ROOT_STAGE_I) {
                std::cout << "[TestEngine]: found existing entry in T.Table: [score: " << relative_score(entry->score, &virtualBoard);
                std::cout << ", type: " << int(entry->typeOfNode) << ", best " << entry->bestMove << "]\n";
            }

            if (entry->depth >= depth || entry->typeOfNode == TERMINAL_NODE)
                return entry->score;
                
            suggestedMove = entry->bestMove;
            eval = entry->score;
        }

        // Go to quiescence if maximum depth reached
        if (depth <= 0)
            return quiescence<Q_ROOT_STAGE>(alpha, beta, MAX_QUIESCENCE_DEPTH);

        // Initial move generation
        MoveSelector moveSelector(&virtualBoard);
        if constexpr (stage == ROOT_STAGE_I || stage == ROOT_STAGE_II) {
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

        // Null move pruning heuristic
        // Note: Since it is used before stealmate condition check, it might make some bugs in extremaly rare positions.
        //       Luckily it does not affect checkmate positions, since it cannot be used when side on move is in check.
        if constexpr (false && nmpAvailable) {
            // Null move pruning failes in zugzwang, so we want to avoid using it in late endgames, where zugzwang positions are most common.
            // Also, we want to prevent it from running twice in a row in the same branch.
            if (!virtualBoard.isInCheck() && virtualBoard.gameStage() > Evaluation::SINGLE_ROOK_VS_ROOK_ENDGAME) {
                static_eval = evaluate();
                int threats = evaluator.getThreatCount(virtualBoard.movingSide());
                Value betterEval = eval != NO_EVAL ? eval : static_eval;

                if (threats == 0 && (depth < NPM_CHECK_MIN_DEPTH || betterEval - NPM_ACTIVATION_THRESHOLD > beta)) {
                    virtualBoard.makeNullMove();

                    // NPM is not available at root node, so nextStage has to be COMMON_STAGE
                    // nmpAvailable = false to prevent double null move in two consecutive nodes
                    Value score = -search<COMMON_STAGE, false>(-beta, -alpha, depth - 1);

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
                
                    // In other case, we can extract a refutation move (best move for other side) and try to improve move ordering with that
                    const TranspositionTable::Entry* npmEntry = tTable->probe(virtualBoard.hash(), virtualBoard.pieces());
                    if (npmEntry) {
                        moveSelector.dFrom = npmEntry->bestMove.from();
                        moveSelector.dTo = npmEntry->bestMove.to();
                    }

                    virtualBoard.undoNullMove();
                }
            }
        }

        // Key state variables
        Value bestScore = -MAX_EVAL;
        NodeType nodeType = ALL_NODE;
        Move bestMove;                  // A null move by default

        // Specify selection strategy according to search stage
        constexpr GenerationStrategy genStrategy = stage == COMMON_STAGE ? GenerationStrategy::CASCADE : GenerationStrategy::STRICT;
        constexpr SelectionStrategy selStrategy = stage == COMMON_STAGE ? SelectionStrategy::STANDARD_ORDERING : SelectionStrategy::SIMPLE;
        constexpr SearchStage nextStage = stage == ROOT_STAGE_I ? ROOT_STAGE_II : COMMON_STAGE;

        const bool improveOrdering = moveSelector.dFrom != INVALID_SQUARE;

        if constexpr (SEARCH_MODE == TRACE && stage == ROOT_STAGE_I)
            std::cout << "Analyzing the following moves:\n";
        
        // Main loop
        Move move = suggestedMove != Move::null() ? suggestedMove : 
                    improveOrdering ? moveSelector.selectNext<genStrategy, SelectionStrategy::IMPROVED_ORDERING>() :
                                      moveSelector.selectNext<genStrategy, selStrategy>();
        while (move != Move::null()) {
            // Make move and evaluate further
            virtualBoard.makeMove(move);
            Value score = -search<nextStage, true>(-beta, -alpha, depth - 1);
            virtualBoard.undoLastMove();

            if constexpr (SEARCH_MODE == TRACE && stage == ROOT_STAGE_I)
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
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
                if (score > alpha) {
                    alpha = score;
                    nodeType = PV_NODE;
                }
            }

            move = moveSelector.selectNext<genStrategy, selStrategy>();

            // Avoid repeating of transposition table suggestion
            if (move != Move::null() && move == suggestedMove)
                move = improveOrdering ? moveSelector.selectNext<genStrategy, SelectionStrategy::IMPROVED_ORDERING>() :
                                         moveSelector.selectNext<genStrategy, selStrategy>();
        }

        // Save search results in transposition table
        tTable->set({
            virtualBoard.hash(),
            virtualBoard.pieces(),
            depth,
            bestScore,
            bestMove,
            nodeType,
            virtualBoard.halfmovesPlain()
        }, rootAge);

        return bestScore;
    }


    // -----------------------------------
    // Crawler methods - quiescence search
    // -----------------------------------

    template <SearchStage stage>
    Value Crawler::quiescence(Value alpha, Value beta, Depth depth)
    {
        qs_nodes++;
        
        const TranspositionTable::Entry* entry = tTable->probe(virtualBoard.hash(), virtualBoard.pieces());
        if (entry)
            return entry->score;

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
        Value standpat = evaluate();

        // Maximum queiescence depth reached
        if (depth == 0)
            return standpat;

        Value bestScore = -MAX_EVAL;
        Move move;

        // Check all "good" captures which could boost the score near the alpha (upper bound) region
        if (moveSelector.currGenType >= MoveGeneration::CAPTURE) {
            // Experimental - sort moves by SEE at initial quiescence depth
            if constexpr (stage == Q_ROOT_STAGE)
                moveSelector.sortCaptures();

            constexpr SelectionStrategy selStrategy = stage == Q_ROOT_STAGE ? SelectionStrategy::SIMPLE : SelectionStrategy::STANDARD_ORDERING;

            move = moveSelector.selectNext<GenerationStrategy::STRICT, selStrategy>();
            while (move.see > 0) {
                // Delta pruning
                if (standpat + move.see + DELTA_MARGIN < alpha && moveSelector.currGenType != MoveGeneration::CHECK_EVASION) {
                    if constexpr (stage == Q_ROOT_STAGE)
                        break;
                    else {
                        move = moveSelector.selectNext<GenerationStrategy::STRICT, selStrategy>();
                        continue;
                    }
                }

                virtualBoard.makeMove(move);
                Value score = -quiescence<Q_COMMON_STAGE>(-beta, -alpha, depth - 1);
                virtualBoard.undoLastMove();

                if (score >= beta)
                    return score;
                
                if (score > bestScore) {
                    bestScore = score;
                    if (score > alpha)
                        alpha = score;
                }
                
                move = moveSelector.selectNext<GenerationStrategy::STRICT, selStrategy>();
            }
        }

        int noThreats = evaluator.getThreatCount(virtualBoard.movingSide());
        Bitboard threats = evaluator.getThreatMap(virtualBoard.movingSide());

        // Consider moving threatened pieces or capture attacking pieces to stabilize
        if (standpat + EPSILON_MARGIN > alpha && (noThreats > 1 || virtualBoard.isInCheck())) {
            if (move == Move::null())
                move = moveSelector.selectNext<GenerationStrategy::CASCADE, SelectionStrategy::SIMPLE>();
            while (move != Move::null()) {
                PieceType pType = type_of(virtualBoard.onSquare(move.to()));
                Bitboard attacks = pType == NULL_TYPE ? 0 :
                                   pType == PAWN ? Pieces::pawn_attacks(~virtualBoard.movingSide(), move.to()) :
                                                   Pieces::piece_attacks_d(pType, move.to(), virtualBoard.pieces());
                if (virtualBoard.isInCheck() ||
                    moveSelector.currGenType != MoveGeneration::CAPTURE && threats & move.from() ||
                    moveSelector.currGenType == MoveGeneration::CAPTURE &&
                    standpat + EPSILON_MARGIN + move.see > alpha &&
                    (threats & move.from() || attacks & threats)
                ) {
                    virtualBoard.makeMove(move);
                    Value score = -quiescence<Q_COMMON_STAGE>(-beta, -alpha, depth - 1);
                    virtualBoard.undoLastMove();

                    if (score >= beta)
                        return score;
                    
                    if (score > bestScore) {
                        bestScore = score;
                        if (score > alpha)
                            alpha = score;
                    }
                }

                move = moveSelector.selectNext<GenerationStrategy::CASCADE, SelectionStrategy::SIMPLE>();
            }

            return bestScore;
        }
        
        return std::max(standpat, bestScore);
    }

}