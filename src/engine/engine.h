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
    Engine() : tTable(), crawler(&this->tTable) {}

    // Setup
    void setPosition(BoardConfig* board);

    // Main functionalities
    Value evaluate(Search::Depth depth);    // No iterative deepening as for now

private:
    // Shared modules
    TranspositionTable tTable;

    // Search crawlers
    Search::Crawler crawler;    // For now we use 1 crawler as we use only 1 thread on search
};