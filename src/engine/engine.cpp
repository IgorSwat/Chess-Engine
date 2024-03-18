#include "engine.h"
#include <numeric>
#include <chrono>


namespace {
    constexpr MoveGenType CHECK_GENERATIONS[1] = {CHECK_EVASION};
    constexpr MoveGenType NO_CHECK_GENERATIONS[3] = {CAPTURE, QUIET_CHECK, QUIET};

    constexpr bool countNodes = true;
}


Engine::Engine(BoardConfig* board)
    : realBoard(board), virtualBoard(), evaluator(&virtualBoard)
{
}

Value Engine::evaluate(Depth depth)
{
    return search(std::numeric_limits<Value>::min() + 1, std::numeric_limits<Value>::max(), depth);
}

Value Engine::search(Value alpha, Value beta, Depth depth)
{
    if constexpr (countNodes)
        nodes++;

    if (depth == 0)
        return evaluate();

    Value bestScore = alpha;
    Move bestMove;
    NodeType typeOfNode = ALL_NODE;

    generateMoves(moveLists[depth]);
    for (int i = 0; i < movegenRange; i++) {
        const MoveList& moves = moveLists[depth][i];
        for (const Move& move : moves) {
            if (!virtualBoard.legalityCheckLight(move))
                continue;
            virtualBoard.makeMove(move);
            Value score = -search(-beta, -alpha, depth - 1);
            virtualBoard.undoLastMove();
            if (score >= beta) {
                transpositionTable.set({virtualBoard.hash(), virtualBoard.pieces(), depth, beta, move, CUT_NODE, virtualBoard.halfmovesPlain()},
                                        realBoard->halfmovesPlain());
                return beta;    // Beta cut-off
            }
            if (score > alpha) {    // PV-node
                alpha = score;
                typeOfNode = PV_NODE;
                bestMove = move;
                bestScore = score;
            }
        }
    }

    transpositionTable.set({virtualBoard.hash(), virtualBoard.pieces(), depth, bestScore, bestMove, typeOfNode, virtualBoard.halfmovesPlain()},
                            realBoard->halfmovesPlain());
    return alpha;
}

void Engine::generateMoves(MoveList* moveLists)
{
    if (virtualBoard.isInCheck(virtualBoard.movingSide())) {
        moveLists[0].clear();
        MoveGeneration::generateMoves<CHECK_EVASION>(moveLists[0], virtualBoard);
        movegenRange = 1;
    }
    else {
        moveLists[0].clear();
        MoveGeneration::generateMoves<CAPTURE>(moveLists[0], virtualBoard);
        /*moveLists[depth][1].clear();
        MoveGeneration::generateMoves<QUIET_CHECK>(moveLists[depth][1], virtualBoard);*/
        moveLists[1].clear();
        MoveGeneration::generateMoves<QUIET>(moveLists[1], virtualBoard);
        movegenRange = 2;
    }
}