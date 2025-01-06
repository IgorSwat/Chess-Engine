#pragma once

#include "search.h"
#include "transpositionTable.h"


// ------------
// Engine class
// ------------

// A top class organizing the entire position assessment process
class Engine
{
public:
    Engine() : crawler(&this->tTable, &this->history) {}

    // Setup
    void setPosition(BoardConfig* board);
    void setPosition(const std::string& fen);

    // Main functionalities
    Value evaluate(Search::Depth depth = 0);    // No iterative deepening as for now. By default returns a static eval (depth 0)
    Value iterativeDeepening(Search::Depth depth);

    // Getters
    const TranspositionTable* transpositionTable() const;

    // [TESTING PURPOSES]
    // TMP
    void showHistory() const;

private:
    // Shared modules
    TranspositionTable tTable;
    Search::History history;

    // Search crawlers
    Search::Crawler crawler;    // For now we use 1 crawler as we use only 1 thread on search
};