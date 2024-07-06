#pragma once

#include "evaluation.h"
#include "moveGeneration.h"
#include "transpositionTable.h"



constexpr int MAX_THREADS = 4;
constexpr Depth MAX_REACHABLE_DEPTH = 100;



class Engine
{
public:
    Engine(BoardConfig* board);

    void reset(const std::string& fen);
    void reset(const BoardConfig* realBoard);

    // Search
    Value evaluate(Depth depth);

private:
    // Basic engine actions
    Value search(Value alpha, Value beta, Depth depth);
    Value evaluate();

    // Other helper functions
    Value relative(Value eval) const;

    // Virtual board to perform search
    BoardConfig virtualBoard;
    // Real board info
    Age currentPositionAge;
    // Move generation containers
    MoveList generatedMoves[MAX_THREADS][MAX_REACHABLE_DEPTH];
    // Evaluation aspects
    Evaluation::Evaluator evaluator;
    // Transposition table
    TranspositionTable transpositionTable;
};


inline void Engine::reset(const std::string& fen)
{
    virtualBoard.loadPosition(fen);
    currentPositionAge = virtualBoard.halfmovesPlain();
}

inline void Engine::reset(const BoardConfig* realBoard)
{
    virtualBoard.loadPosition(*realBoard);
    currentPositionAge = virtualBoard.halfmovesPlain();
}

inline Value Engine::relative(Value eval) const
{
    return virtualBoard.movingSide() == WHITE ? eval : -eval;
}

inline Value Engine::evaluate()
{
    // Returns a relative evaluation to fit in the NegaMax architecture
    return relative(evaluator.evaluate());
}