#pragma once

#include "misc.h"


constexpr Bitboard FILE_A_BB = 0x0101010101010101;
constexpr Bitboard FILE_B_BB = FILE_A_BB << 1;
constexpr Bitboard FILE_C_BB = FILE_A_BB << 2;
constexpr Bitboard FILE_D_BB = FILE_A_BB << 3;
constexpr Bitboard FILE_E_BB = FILE_A_BB << 4;
constexpr Bitboard FILE_F_BB = FILE_A_BB << 5;
constexpr Bitboard FILE_G_BB = FILE_A_BB << 6;
constexpr Bitboard FILE_H_BB = FILE_A_BB << 7;
constexpr Bitboard FILES[8]{
	FILE_A_BB, FILE_B_BB, FILE_C_BB, FILE_D_BB, FILE_E_BB, FILE_F_BB, FILE_G_BB, FILE_H_BB
};

constexpr Bitboard CENTRAL_FILES = FILE_C_BB | FILE_D_BB | FILE_E_BB | FILE_F_BB;
constexpr Bitboard BG_FILES = FILE_B_BB | FILE_G_BB;
constexpr Bitboard EDGE_FILES = FILE_A_BB | FILE_H_BB;

constexpr Bitboard ROW_1 = 0xFF;
constexpr Bitboard ROW_2 = ROW_1 << (8 * 1);
constexpr Bitboard ROW_3 = ROW_1 << (8 * 2);
constexpr Bitboard ROW_4 = ROW_1 << (8 * 3);
constexpr Bitboard ROW_5 = ROW_1 << (8 * 4);
constexpr Bitboard ROW_6 = ROW_1 << (8 * 5);
constexpr Bitboard ROW_7 = ROW_1 << (8 * 6);
constexpr Bitboard ROW_8 = ROW_1 << (8 * 7);
constexpr Bitboard ROWS[8]{
	ROW_1, ROW_2, ROW_3, ROW_4, ROW_5, ROW_6, ROW_7, ROW_8
};

constexpr Bitboard NOT_FILE_A = ~FILE_A_BB;
constexpr Bitboard NOT_FILE_H = ~FILE_H_BB;
constexpr Bitboard NOT_FILE_AB = ~(FILE_A_BB | FILE_B_BB);
constexpr Bitboard NOT_FILE_GH = ~(FILE_G_BB | FILE_H_BB);
constexpr Bitboard NOT_ROW_1 = ~ROW_1;
constexpr Bitboard NOT_ROW_8 = ~ROW_8;
constexpr Bitboard NOT_ROW_12 = ~(ROW_1 & ROW_2);
constexpr Bitboard NOT_ROW_78 = ~(ROW_7 & ROW_8);


constexpr Bitboard DIAG_A1H8 = 0x8040201008040201;
constexpr Bitboard DIAG_A8H1 = 0x0102040810204080;

constexpr Bitboard DARK_SQUARES_BB = 0xaa55aa55aa55aa55;
constexpr Bitboard LIGHT_SQUARES_BB = 0x55aa55aa55aa55aa;


extern int SQUARE_DISTANCE[SQUARE_RANGE][SQUARE_RANGE];
extern Bitboard PATHS_BETWEEN[SQUARE_RANGE][SQUARE_RANGE];		// Paths between each pair of squares mapped to bitboards
extern Bitboard LINES[SQUARE_RANGE][SQUARE_RANGE];				// Lines containing two given squares (superset of PATHS_BETWEEN)
extern Bitboard ADJACENT_RANK_SQUARES[EXTENDED_SQUARE_RANGE];
extern Bitboard ADJACENT_FILES[SQUARE_RANGE];
extern Bitboard PATHS_TO_CENTRAL_FILES[SQUARE_RANGE];



void initBoardElements();

constexpr inline Bitboard fileBB(int file)
{
	return FILES[file];
}

constexpr inline Bitboard rankBB(int rank)
{
	return ROWS[rank];
}

constexpr inline Bitboard fileBBOf(Square s)
{
	return FILES[fileOf(s)];
}

constexpr inline Bitboard rankBBOf(Square s)
{
	return ROWS[rankOf(s)];
}

constexpr inline Bitboard squaresOfColor(SquareColor color)
{
	return color == DARK_SQUARE ? DARK_SQUARES_BB : LIGHT_SQUARES_BB;
}

constexpr inline Bitboard pathBetween(Square sq1, Square sq2)
{
	return PATHS_BETWEEN[sq1][sq2];
}

constexpr inline Bitboard lineWith(Square sq1, Square sq2)
{
	return LINES[sq1][sq2];
}

constexpr inline Bitboard adjacentRankSquares(Square sq)
{
	return ADJACENT_RANK_SQUARES[sq];
}

constexpr inline Bitboard adjacentFiles(Square sq)
{
	return ADJACENT_FILES[sq];
}

constexpr inline Bitboard adjacentFiles(int file)
{
	return ADJACENT_FILES[file];
}

constexpr inline bool aligned(Square sq1, Square sq2)
{
	return LINES[sq1][sq2];
}

constexpr inline bool aligned(Square sq1, Square sq2, Square midd)
{
	return LINES[sq1][sq2] & midd;
}