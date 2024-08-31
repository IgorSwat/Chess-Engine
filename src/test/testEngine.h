#pragma once

#include "../engine/search.h"
#include "../engine/transpositionTable.h"


// --------------
// Helper defines
// --------------

struct SearchResult
{
    Value score;
    Search::NodeType nodeType;
    Move bestMove;
};


// ----------------
// TestEngine class
// ----------------

// A pure test class which imitates the behavior of main Engine class with one search crawler, with additional logging
class TestEngine
{
public:
    TestEngine() : virtualBoard(), evaluator(&this->virtualBoard), tTable() {}

    // Setup
    void setPosition(BoardConfig* board);
    void setPosition(const std::string& fen);

    Value evaluate_d(Search::Depth depth);  // With search / queiescence
    Value evaluate_s();                     // Just a static eval

private:
    // All search methods packed together
    Value evaluate();
    template <Search::SearchStage stage>
    SearchResult search(Value alpha, Value beta, Search::Depth depth);     // Recursive subroutine
    template <Search::SearchStage stage, bool trace = false>
    SearchResult quiescence(Value alpha, Value beta, Search::Depth depth);

    // Virtual board and related properties
    BoardConfig virtualBoard;
    Search::Age rootAge = 0;
    Search::Depth lastUsedDepth = 0;

    // Main components
    Evaluation::Evaluator evaluator;
    TranspositionTable tTable;
};


// ------------------
// TestEngine methods
// ------------------

inline Value TestEngine::evaluate()
{
    return Search::relative_score(evaluator.evaluate(), &virtualBoard);
}