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


    // -----------------------------
    // Crawler methods - main search
    // -----------------------------

    Value Crawler::search(Depth depth)
    {
        if (depth == 0)
            return evaluate();

        return search<ROOT_STAGE_I>(-MAX_EVAL, MAX_EVAL, depth);
    }

    template <SearchStage stage>
    Value Crawler::search(Value alpha, Value beta, Depth depth)
    {
        // Check the transposition table
        const TranspositionTable::Entry* entry = tTable->probe(virtualBoard.hash(), virtualBoard.pieces());
        Move suggestedMove;
        if (entry) {
            if (entry->depth >= depth || entry->typeOfNode == TERMINAL_NODE)
                return entry->score;
                
            suggestedMove = entry->bestMove;
        }

        if (depth == 0)
            //return 0;                 // - to test max speed with no eval and beta-cutoffs almost everywhere
            //return gen.random();
            //return evaluate();
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

        // Key state variables
        Value bestScore = -MAX_EVAL;
        NodeType nodeType = ALL_NODE;
        Move bestMove;                  // A null move by default

        // Specify selection strategy according to search stage
        constexpr GenerationStrategy genStrategy = stage == COMMON_STAGE ? GenerationStrategy::CASCADE : GenerationStrategy::STRICT;
        constexpr SelectionStrategy selStrategy = stage == COMMON_STAGE ? SelectionStrategy::STANDARD_ORDERING : SelectionStrategy::SIMPLE;
        constexpr SearchStage nextStage = stage == ROOT_STAGE_I ? ROOT_STAGE_II : COMMON_STAGE;
        
        // Main loop
        Move move = suggestedMove != Move::null() ? suggestedMove : 
                                                    moveSelector.selectNext<genStrategy, selStrategy>();
        while (move != Move::null()) {
            // Make move and evaluate further
            virtualBoard.makeMove(move);
            Value score = -search<nextStage>(-beta, -alpha, depth - 1);
            virtualBoard.undoLastMove();

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
                return beta;
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
                move = moveSelector.selectNext<genStrategy, selStrategy>();
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