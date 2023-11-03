#include "bitboards.h"
#include <cmath>
#include <numeric>


Bitboard PATHS[SQUARE_RANGE][SQUARE_RANGE] = {};
Bitboard ADJACENT_RANK_SQUARES[EXTENDED_SQUARE_RANGE] = {};


void initBoardElements()
{
	// Initialising paths between squares
	for (Square sq1 = SQ_A1; sq1 <= SQ_H8; ++sq1) {
		Bitboard sq1BB = squareToBB(sq1);

		// Initialise the paths between squares
		PATHS[sq1][sq1] = sq1BB;
		for (Square sq2 = sq1 + 1; sq2 <= SQ_H8; ++sq2) {
			Bitboard sq2BB = squareToBB(sq2);
			int xDiff = fileOf(sq1) - fileOf(sq2);
			int yDiff = rankOf(sq1) - rankOf(sq2);
			int d = std::gcd(std::abs(xDiff), std::abs(yDiff));
			xDiff /= d, yDiff /= d;
			if (std::abs(xDiff) < 2 && std::abs(yDiff) < 2) {
				Direction dir = vectorToDir(xDiff, yDiff);
				while ((sq2BB & sq1BB) == 0)
					sq2BB |= Bitboards::shift(sq2BB, dir);
				PATHS[sq1][sq2] = sq2BB;
				PATHS[sq2][sq1] = sq2BB;
			}
		}

		// Initialise adjacent squares
		ADJACENT_RANK_SQUARES[sq1] = Bitboards::shift(sq1BB, WEST) | Bitboards::shift(sq1BB, EAST);
	}
	ADJACENT_RANK_SQUARES[INVALID_SQUARE] = 0;
}