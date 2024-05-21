#include "engine.h"
#include <numeric>

namespace {

}


Engine::Engine(BoardConfig* board)
    : virtualBoard(), evaluator(&virtualBoard)
{
}

Value Engine::evaluate(Depth depth)
{
    return search(std::numeric_limits<Value>::min() + 1, std::numeric_limits<Value>::max(), depth);
}

Value Engine::search(Value alpha, Value beta, Depth depth)
{
    if (depth == 0)
        return evaluate();

    Value bestScore = alpha;
    Move bestMove;
    NodeType typeOfNode = ALL_NODE;

    for (int i = 0; i < 1; i++) {   // TODO: fix
        //const MoveList& generatedMoves = generatedMoves[depth][i];
        //for (const Move& move : generatedMoves) {
        //    if (!virtualBoard.legalityCheckLight(move))
        //        continue;
        //    virtualBoard.makeMove(move);
        //    Value score = -search(-beta, -alpha, depth - 1);
        //    virtualBoard.undoLastMove();
        //    if (score >= beta) {
        //        transpositionTable.set({virtualBoard.hash(), virtualBoard.pieces(), depth, beta, move, CUT_NODE, virtualBoard.halfmovesPlain()},
        //                                currentPositionAge);
        //        return beta;    // Beta cut-off
        //    }
        //    if (score > alpha) {    // PV-node
        //        alpha = score;
        //        typeOfNode = PV_NODE;
        //        bestMove = move;
        //        bestScore = score;
        //    }
        //}
    }

    transpositionTable.set({virtualBoard.hash(), virtualBoard.pieces(), depth, bestScore, bestMove, typeOfNode, virtualBoard.halfmovesPlain()},
                            currentPositionAge);
    return alpha;
}