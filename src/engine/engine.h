#pragma once

#include "evaluation.h"
#include "moveGeneration.h"
#include "transpositionTable.h"


class Engine
{
public:
    Engine(BoardConfig* board);

    void reset();

    // Search
    Value evaluate(Depth depth);

    // Testing & others
    void showBestLine();
    void speedTest(Depth depth);

    static constexpr Depth MAX_REACHABLE_DEPTH = 10;

private:
    Value search(Value alpha, Value beta, Depth depth);
    Value evaluate();
    void generateMoves(MoveList* lists);

    BoardConfig* realBoard;
    BoardConfig virtualBoard;

    MoveList moveLists[MAX_REACHABLE_DEPTH][MAX_USED_CATEGORIES];
    Evaluation::Evaluator evaluator;
    TranspositionTable transpositionTable;

    int movegenRange = 0;
    std::uint64_t nodes = 0;
};


inline void Engine::reset()
{
    virtualBoard.loadFromConfig(*realBoard);
}

inline Value Engine::evaluate()
{
    // Returns a relative evaluation to fit in the NegaMax architecture
    return virtualBoard.movingSide() == WHITE ? evaluator.evaluate() : -evaluator.evaluate();
}