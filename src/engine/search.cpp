#include "search.h"
#include "transpositionTable.h"
#include <numeric>


namespace Search {

    // ---------------
    // Crawler methods
    // ---------------

    Value Crawler::search(Depth depth)
    {
        static constexpr Value MAX_BETA = std::numeric_limits<Value>::max();
        static constexpr Value MIN_ALPHA = -MAX_BETA;

        return search(MIN_ALPHA, MAX_BETA, depth);
    }

    Value Crawler::search(Value alpha, Value beta, Depth depth)
    {
        if (depth == 0)
            return evaluate();
        //  return quiescence() ...     -- needs to cover checkmates and stealmates

        Value bestScore = alpha;
        NodeType nodeType = ALL_NODE;
        Move bestMove;

        // Initial move generation
        MoveSelector moveSelector(&virtualBoard);
        if (virtualBoard.isInCheck(virtualBoard.movingSide()))
            moveSelector.generateMoves<MoveGeneration::CHECK_EVASION>();
        else
            moveSelector.generateMoves<MoveGeneration::CAPTURE>();
        
        // Main loop
        Move move = moveSelector.selectNext<GenerationStrategy::CASCADE, SelectionStrategy::STANDARD_ORDERING>();
        while (move != Move::null()) {
            // Make move and evaluate further
            virtualBoard.makeMove(move);
            Value score = -search(-beta, -alpha, depth - 1);
            virtualBoard.undoLastMove();

            // Beta cut-off: an enemy would never allow a line worse than beta to occur
            if (score >= beta) {
                tTable->set({
                    virtualBoard.hash(),
                    virtualBoard.pieces(),
                    depth,
                    beta,  // score
                    move,
                    CUT_NODE,
                    virtualBoard.halfmovesPlain()
                }, rootAge);
                return beta;
            }

            // PV node - a move that is better for moving side than previous best one
            if (score > alpha) {
                alpha = score;
                nodeType = PV_NODE;
                bestMove = move;
                bestScore = score;
            }

            move = moveSelector.selectNext<GenerationStrategy::CASCADE, SelectionStrategy::STANDARD_ORDERING>();
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

        return alpha;
    }

}