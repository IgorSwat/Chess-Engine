#include "bitboards.h"
#include <cmath>
#include <numeric>


namespace Board {

	// ------------------------------
	// Precalculated board properties
	// ------------------------------

	int SquareDistance[SQUARE_RANGE][SQUARE_RANGE] = { 0 };

	Bitboard Paths[SQUARE_RANGE][SQUARE_RANGE] = { 0 };
	Bitboard Boxes[SQUARE_RANGE][SQUARE_RANGE] = { 0 };
	Bitboard AdjacentFiles[SQUARE_RANGE] = { 0 };
	Bitboard AdjacentRankSquares[EXTENDED_SQUARE_RANGE] = { 0 };

	Bitboard FrontSpan[SQUARE_RANGE][COLOR_RANGE];


	// -----------------------
	// Board-related functions
	// -----------------------

	void initBoardElements()
	{
		// Initialising paths between squares
		for (int sq1 = SQ_A1; sq1 <= SQ_H8; sq1++) {
			Bitboard sq1BB = square_to_bb(Square(sq1));

			// Initialize the paths between squares
			Paths[sq1][sq1] = sq1BB;
			Boxes[sq1][sq1] = sq1BB;
			for (int sq2 = sq1 + 1; sq2 <= SQ_H8; sq2++) {
				Bitboard sq2BB = square_to_bb(Square(sq2));
				int xDiff = file_of(Square(sq1)) - file_of(Square(sq2));
				int yDiff = rank_of(Square(sq1)) - rank_of(Square(sq2));

				int d = std::gcd(std::abs(xDiff), std::abs(yDiff));
				xDiff /= d, yDiff /= d;
				if (std::abs(xDiff) < 2 && std::abs(yDiff) < 2) {
					Direction dir = xDiff == 0 ? (yDiff > 0 ? NORTH : SOUTH) :
						yDiff == 0 ? (xDiff > 0 ? EAST : WEST) :
						xDiff > 0 ? (yDiff > 0 ? NORTH_EAST : SOUTH_EAST) :
						yDiff > 0 ? NORTH_WEST : SOUTH_WEST;

					while ((sq2BB & sq1BB) == 0)
						sq2BB |= Bitboards::shift_d(sq2BB, dir);
					Paths[sq1][sq2] = sq2BB;
					Paths[sq2][sq1] = sq2BB;
				}

				// Initialize square distance
				SquareDistance[sq1][sq2] = std::max(std::abs(file_of(Square(sq1)) - file_of(Square(sq2))), std::abs(rank_of(Square(sq1)) - rank_of(Square(sq2))));
				SquareDistance[sq2][sq1] = SquareDistance[sq1][sq2];

				// Initialize boxes
				Bitboard fill1 = file_of(Square(sq2)) >= file_of(Square(sq1)) ? Bitboards::fill<EAST>(square_to_bb(Square(sq1))) : Bitboards::fill<WEST>(square_to_bb(Square(sq1)));
				fill1 = rank_of(Square(sq2)) >= rank_of(Square(sq1)) ? Bitboards::fill<NORTH>(fill1) : Bitboards::fill<SOUTH>(fill1);
				Bitboard fill2 = file_of(Square(sq1)) > file_of(Square(sq2)) ? Bitboards::fill<EAST>(square_to_bb(Square(sq2))) : Bitboards::fill<WEST>(square_to_bb(Square(sq2)));
				fill2 = rank_of(Square(sq1)) > rank_of(Square(sq2)) ? Bitboards::fill<NORTH>(fill2) : Bitboards::fill<SOUTH>(fill2);
				Boxes[sq1][sq2] = fill1 & fill2;
				Boxes[sq2][sq1] = fill1 & fill2;
			}

			// Initialize adjacent rank squares
			AdjacentRankSquares[sq1] = Bitboards::shift_s<WEST>(sq1BB) | Bitboards::shift_s<EAST>(sq1BB);

			// Initialize adjacent files bitboards
			Bitboard adjacentFiles = 0;
			if (file_of(Square(sq1)) != 0) adjacentFiles |= Files[file_of(Square(sq1)) - 1];
			if (file_of(Square(sq1)) != 7) adjacentFiles |= Files[file_of(Square(sq1)) + 1];
			AdjacentFiles[sq1] = adjacentFiles;

			// Initialize front span bitboards
			Bitboard rankArea = AdjacentRankSquares[sq1] | Square(sq1);
			FrontSpan[sq1][WHITE] = Bitboards::fill<NORTH>(rankArea) ^ rankArea;
			FrontSpan[sq1][BLACK] = Bitboards::fill<SOUTH>(rankArea) ^ rankArea;
		}

		AdjacentRankSquares[INVALID_SQUARE] = 0;
	}

}