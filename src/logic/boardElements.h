#pragma once

#include "types.h"
#include <cstdlib>
#include <algorithm>


namespace Board {

	// ------------------------
	// Board geometry bitboards
	// ------------------------

	constexpr Bitboard FILE_A = 0x0101010101010101;
	constexpr Bitboard FILE_B = FILE_A << 1;
	constexpr Bitboard FILE_C = FILE_A << 2;
	constexpr Bitboard FILE_D = FILE_A << 3;
	constexpr Bitboard FILE_E = FILE_A << 4;
	constexpr Bitboard FILE_F = FILE_A << 5;
	constexpr Bitboard FILE_G = FILE_A << 6;
	constexpr Bitboard FILE_H = FILE_A << 7;
	constexpr Bitboard Files[8] = { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H };

	constexpr Bitboard CENTRAL_FILES = FILE_C | FILE_D | FILE_E | FILE_F;
	constexpr Bitboard BG_FILES = FILE_B | FILE_G;
	constexpr Bitboard EDGE_FILES = FILE_A | FILE_H;
	constexpr Bitboard NOT_FILE_A = ~FILE_A;
	constexpr Bitboard NOT_FILE_H = ~FILE_H;
	constexpr Bitboard NOT_FILE_AB = ~(FILE_A | FILE_B);
	constexpr Bitboard NOT_FILE_GH = ~(FILE_G | FILE_H);
	constexpr Bitboard NOT_EDGE_FILES = ~EDGE_FILES;

	constexpr Bitboard RANK_1 = 0xff;
	constexpr Bitboard RANK_2 = RANK_1 << (8 * 1);
	constexpr Bitboard RANK_3 = RANK_1 << (8 * 2);
	constexpr Bitboard RANK_4 = RANK_1 << (8 * 3);
	constexpr Bitboard RANK_5 = RANK_1 << (8 * 4);
	constexpr Bitboard RANK_6 = RANK_1 << (8 * 5);
	constexpr Bitboard RANK_7 = RANK_1 << (8 * 6);
	constexpr Bitboard RANK_8 = RANK_1 << (8 * 7);
	constexpr Bitboard Ranks[8] = { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8 };

	constexpr Bitboard CENTRAL_RANKS = RANK_3 | RANK_4 | RANK_5 | RANK_6;
	constexpr Bitboard NOT_RANK_1 = ~RANK_1;
	constexpr Bitboard NOT_RANK_8 = ~RANK_8;
	constexpr Bitboard NOT_RANK_12 = ~(RANK_1 & RANK_2);
	constexpr Bitboard NOT_RANK_78 = ~(RANK_7 & RANK_8);
	
	constexpr Bitboard DIAG_A1H8 = 0x8040201008040201;
	constexpr Bitboard DIAG_A8H1 = 0x0102040810204080;

	constexpr Bitboard DARK_SQUARES = 0xaa55aa55aa55aa55;
	constexpr Bitboard LIGHT_SQUARES = 0x55aa55aa55aa55aa;

	constexpr Bitboard QUEENSIDE = FILE_A | FILE_B | FILE_C | FILE_D;
	constexpr Bitboard KINGSIDE = FILE_E | FILE_F | FILE_G | FILE_H;

	constexpr Bitboard BOARD = 0xffffffffffffffff;


	// ------------------------------
	// Precalculated board properties
	// ------------------------------

	extern int SquareDistance[SQUARE_RANGE][SQUARE_RANGE];			// Maximum metric between two squares file and rank distances

	extern Bitboard Paths[SQUARE_RANGE][SQUARE_RANGE];				// Paths between each pair of squares mapped to bitboards
	extern Bitboard Boxes[SQUARE_RANGE][SQUARE_RANGE];
	extern Bitboard AdjacentFiles[SQUARE_RANGE];
	extern Bitboard AdjacentRankSquares[EXTENDED_SQUARE_RANGE];

	// A front span is an area composed of 3 adjacent files in front of given square, where direction is defined by given side (WHITE = NORTH, BLACK = SOUTH).
	// It's being used mainly for pawn structure geometry calculations.
	//	  ...
	//	|x|x|x|
	//	|x|x|x|		- an example of a front span of square denoted as P
	//	| |P| |

	extern Bitboard FrontSpan[SQUARE_RANGE][COLOR_RANGE];


	// -----------------------
	// Board-related functions
	// -----------------------

	void initBoardElements();

	constexpr inline Bitboard file_bb_of(Square sq)
	{
		return Files[file_of(sq)];
	}

	constexpr inline Bitboard rank_bb_of(Square sq)
	{
		return Ranks[rank_of(sq)];
	}

	inline bool aligned(Square sq1, Square sq2)
	{
		return Paths[sq1][sq2];
	}

	// Important note: we assume that from is the first square in line, that means either from < to1 < to2 or from < to1 < to2
	inline bool aligned(Square from, Square to1, Square to2)
	{
		return (Paths[from][to2] & to1) || (Paths[from][to1] & to2);
	}

	template <Color side>
	inline Bitboard back_span(Square sq)						// A reverse of a front span of given square
	{															//	| |P| |
		constexpr Color enemy = ~side;							//	|x|x|x|
		return FrontSpan[sq][enemy];							//	|x|x|x|
	}															//	  ...

	template <Color side>
	inline Bitboard back_span_m(Square sq)						// A mild back span with addition of adjacent rank squares
	{															//	|x|P|x|
		return back_span<side>(sq) | AdjacentRankSquares[sq];	//	|x|x|x|
	}															//	|x|x|x|

	// TODO: Remove it
	constexpr inline Bitboard squares_of_color(SquareColor color)
	{
		return color == DARK_SQUARE ? DARK_SQUARES : LIGHT_SQUARES;
	}

}