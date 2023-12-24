#pragma once

#include "pgnParser.h"
#include "../logic/zobrist.h"

class PGNZobristTester : public PGNParser
{
public:
	PGNZobristTester(BoardConfig* board) : PGNParser(board) { zobrist.generateHash(board); }

	bool applyNextMove() override;

private:
	void testZobrist();

	Zobrist::ZobristHash zobrist;
};

