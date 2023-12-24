#include "pgnZobristTester.h"
#include "../logic/boardConfig.h"
#include <cassert>
#include <iomanip>

bool PGNZobristTester::applyNextMove()
{
    bool result = PGNParser::applyNextMove();
    testZobrist();
    return result;
}

void PGNZobristTester::testZobrist()
{
    zobrist.generateHash(board);
    std::cout << std::dec << board->hash() << std::endl;
    assert(zobrist.getHash() == board->hash());
}