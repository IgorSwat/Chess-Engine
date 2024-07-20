#include "engine.h"


// --------------
// Engine methods
// --------------

void Engine::setPosition(BoardConfig* board)
{
    crawler.setPosition(board);
}

// Main search framework
Value Engine::evaluate(Search::Depth depth)
{
    // Temporary
    return crawler.search(depth);
}