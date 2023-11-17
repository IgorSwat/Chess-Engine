#include "bitboards.h"
#include <cmath>
#include <numeric>


Bitboard PATHS_BETWEEN[SQUARE_RANGE][SQUARE_RANGE] = { 0 };
Bitboard LINES[SQUARE_RANGE][SQUARE_RANGE] = { 0 };
Bitboard ADJACENT_RANK_SQUARES[EXTENDED_SQUARE_RANGE] = { 0 };
Bitboard PATHS_TO_CENTRAL_FILES[SQUARE_RANGE] = { 0 };


void initBoardElements()
{
	// Initialising paths between squares
	for (Square sq1 = SQ_A1; sq1 <= SQ_H8; ++sq1) {
		Bitboard sq1BB = squareToBB(sq1);

		// Initialise the paths between squares
		PATHS_BETWEEN[sq1][sq1] = sq1BB;
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
				PATHS_BETWEEN[sq1][sq2] = sq2BB;
				PATHS_BETWEEN[sq2][sq1] = sq2BB;

				Bitboard forwardBB = squareToBB(sq2);
				Bitboard backwardBB = squareToBB(sq2);
				Bitboard lineBB = 0;
				while (forwardBB || backwardBB) {
					lineBB |= forwardBB;
					lineBB |= backwardBB;
					forwardBB = Bitboards::shift(forwardBB, dir);
					backwardBB = Bitboards::shift(backwardBB, ~dir);
				}
				LINES[sq1][sq2] = lineBB;
				LINES[sq2][sq1] = lineBB;
			}
		}

		// Initialise adjacent squares
		ADJACENT_RANK_SQUARES[sq1] = Bitboards::shift(sq1BB, WEST) | Bitboards::shift(sq1BB, EAST);

		// Initialise paths to central files
		Direction dir = fileOf(sq1) < E_FILE ? EAST : WEST;
		Bitboard path = 0;
		Square sq2 = sq1 + dir;
		while (fileOf(sq2) != D_FILE && fileOf(sq2) != E_FILE) {
			path |= sq2;
			sq2 = sq2 + dir;
		}
		PATHS_TO_CENTRAL_FILES[sq1] = path;
	}
	ADJACENT_RANK_SQUARES[INVALID_SQUARE] = 0;
}