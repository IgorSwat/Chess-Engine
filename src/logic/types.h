#pragma once

#include <cinttypes>
#include <cctype>


using Bitboard = uint64_t;


// -----------------
// Squares and files
// -----------------

enum File : int {
	A_FILE = 0, B_FILE, C_FILE, D_FILE, E_FILE, F_FILE, G_FILE, H_FILE,

	FILE_RANGE = 8
};

enum Square : int {
	SQ_A1 = 0, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
	SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
	SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
	SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
	SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
	SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
	SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
	SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
	INVALID_SQUARE = 64,

	SQUARE_RANGE = 64,
	EXTENDED_SQUARE_RANGE = 65
};

constexpr inline File file_of(Square s)
{
	return File(s & 0x7);
}

constexpr inline int rank_of(Square s)
{
	return s >> 3;
}

constexpr inline Square make_square(int rank, int file)
{
	// We assume that 0 <= rank < 8 and 0 <= file < 8
	return Square((rank << 3) | file);
}

// Operators for bitboard-square interactions
constexpr inline Bitboard square_to_bb(Square s)
{
	return 1ULL << s;
}

constexpr inline Bitboard operator~(Square sq)
{
	return ~square_to_bb(sq);
}

constexpr inline Bitboard operator&(Bitboard bb, Square sq)
{
	return bb & square_to_bb(sq);
}

inline Bitboard operator&=(Bitboard& bb, Square sq)
{
	bb &= square_to_bb(sq);
	return bb;
}

constexpr inline Bitboard operator|(Bitboard bb, Square sq)
{
	return bb | square_to_bb(sq);
}

inline Bitboard operator|=(Bitboard& bb, Square sq)
{
	bb |= square_to_bb(sq);
	return bb;
}

constexpr inline Bitboard operator^(Bitboard bb, Square sq)
{
	return bb ^ square_to_bb(sq);
}

inline Bitboard operator^=(Bitboard& bb, Square sq)
{
	bb ^= square_to_bb(sq);
	return bb;
}


// -------------
// Square colors
// -------------

enum SquareColor : int {
	DARK_SQUARE = 0, LIGHT_SQUARE,

	SQUARE_COLOR_RANGE = 2
};

constexpr inline SquareColor operator~(SquareColor color)
{
	return SquareColor(color ^ LIGHT_SQUARE);
}

constexpr inline SquareColor color_of(Square s)
{
	return SquareColor(((s >> 3) & 0x1) ^ (s & 0x1));
}


// ----------
// Directions
// ----------

enum Direction : int {
	WEST = 0, SOUTH_WEST, SOUTH, SOUTH_EAST, EAST, NORTH_EAST, NORTH, NORTH_WEST,
	INVALID_DIRECTION = 8,

	POSITIVE_DIRECTIONS = 4,
	DIRECTION_RANGE = 8
};

constexpr int DIRECTION_SHIFTS[DIRECTION_RANGE] = { -1, -9, -8, -7, 1, 9, 8, 7 };

constexpr inline Direction operator~(Direction dir)
{
	return Direction((int(dir) + 4) & 0x7);
}

constexpr inline Square operator+(Square sq, Direction dir)
{
	return Square(sq + DIRECTION_SHIFTS[dir]);
}

constexpr inline Square operator-(Square sq, Direction dir)
{
	return Square(sq - DIRECTION_SHIFTS[dir]);
}


// -------------
// Playing sides
// -------------

enum Color : int {
	WHITE = 0, BLACK,

	COLOR_RANGE = 2
};

constexpr inline Color operator~(Color side)
{
	return Color(side ^ BLACK);
}


// ------
// Pieces
// ------

enum PieceType : int {
	NULL_TYPE = 0,

	PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
	ALL_PIECES,

	PIECE_TYPE_RANGE = 8
};

enum Piece : int {
	NO_PIECE = 0,

	W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,

	BLACK_PIECE = 8,
	B_PAWN = W_PAWN | BLACK_PIECE, 
	B_KNIGHT = W_KNIGHT | BLACK_PIECE, 
	B_BISHOP = W_BISHOP | BLACK_PIECE, 
	B_ROOK = W_ROOK | BLACK_PIECE, 
	B_QUEEN = W_QUEEN | BLACK_PIECE,
	B_KING = W_KING | BLACK_PIECE,

	PIECE_RANGE = 16,
};

constexpr inline Color color_of(Piece piece)
{
	return Color(piece >> 3);
}

constexpr inline PieceType type_of(Piece piece)
{
	return PieceType(piece & 0x7);
}

constexpr inline Piece make_piece(Color color, PieceType type)
{
	return Piece(type | (color << 3));
}


// --------
// Castling
// --------

enum CastlingRights : int {
	NO_CASTLING = 0,

	WHITE_OO = 1,
	WHITE_OOO = WHITE_OO << 1,
	BLACK_OO = WHITE_OO << 2,
	BLACK_OOO = WHITE_OO << 3,

	KINGSIDE_CASTLE = WHITE_OO | BLACK_OO,
	QUEENSIDE_CASTLE = WHITE_OOO | BLACK_OOO,
	WHITE_BOTH = WHITE_OO | WHITE_OOO,
	BLACK_BOTH = BLACK_OO | BLACK_OOO,
	NO_WHITE_OO = WHITE_OOO | BLACK_BOTH,
	NO_WHITE_OOO = WHITE_OO | BLACK_BOTH,
	NO_BLACK_OO = WHITE_BOTH | BLACK_OOO,
	NO_BLACK_OOO = WHITE_BOTH | BLACK_OO,
	ALL_RIGHTS = WHITE_BOTH | BLACK_BOTH,

	CASTLING_RIGHTS_RANGE = 16
};

using CastleType = CastlingRights;

constexpr inline CastleType make_castle_type(Color side, CastlingRights castling)
{
	return side == WHITE ? CastleType(castling & WHITE_BOTH) : CastleType(castling & BLACK_BOTH);
}
