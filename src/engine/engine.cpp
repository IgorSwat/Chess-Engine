#include "engine.h"


// --------------
// Engine methods
// --------------

void Engine::setPosition(BoardConfig* board)
{
    crawler.setPosition(board);
}

void Engine::setPosition(const std::string& fen)
{
    crawler.setPosition(fen);
}

// Returns a relative score - to transform it back to absolute score you need to apply relative_score() again
Value Engine::evaluate(Search::Depth depth)
{
    return crawler.search(depth);
}

const TranspositionTable* Engine::transpositionTable() const
{
    return &tTable;
}