#include "bitboards.h"
#include <sstream>
#include <cmath>
#include <numeric>


Bitboard PATHS[SQUARE_RANGE][SQUARE_RANGE] = {};

namespace Bitboards {
	std::string bitboardToString(Bitboard bb)
	{
		int bb2D[8][8];
		int i = 7, j = 0;
		for (int k = 0; k < 64; k++)
		{
			bb2D[i][j] = bb % 2;
			bb >>= 1;
			j++;
			if (j == 8)
			{
				i--;
				j = 0;
			}
		}
		std::ostringstream stream;
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 8; j++)
				stream << bb2D[i][j];
			if (i != 7)
				stream << std::endl;
		}
		return stream.str();
	}
}


void initBoardElements()
{
	// Initialising paths between squares
	for (Square sq1 = SQ_A1; sq1 <= SQ_H8; ++sq1) {
		Bitboard sq1BB = squareToBB(sq1);
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
	}
}