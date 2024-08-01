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

Value Engine::evaluate(Search::Depth depth)
{
    return crawler.search(depth);
}

const TranspositionTable* Engine::transpositionTable() const
{
    return &tTable;
}