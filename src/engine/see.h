#pragma once

#include "../logic/boardConfig.h"


// Static exchange evaluator class
// It's main responsibility is to evaluate material gain or loss in given exchange sequence on given board
class SEE
{
public:
	SEE(BoardConfig* board) : board(board) {}

	int evaluate(Square from, Square to);
	int evaluate(const Move& move);

private:
	// Least valuable piece - returns a single populated Bitboard containing the least valuable piece square (or empty if no piece exists in given area)
	Bitboard lvp(Color side, Bitboard area, PieceType& type) const;

	static constexpr int MAX_DEPTH = 33;

	BoardConfig* board;
	int gain[MAX_DEPTH] = { 0 };
};


inline int SEE::evaluate(const Move& move)
{
	return evaluate(move.from(), move.to());
}