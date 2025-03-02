#pragma once

#include <cinttypes>
#include <cstring>
#include <iostream>


/*
    ---------- Types ----------

    Common types definitions shared among all engine modules.
    - Consolidates basic definitions into one header file to address potential issues of including order
    - Defines basic board geometry types (squares, ranks, files, directions)
    - Defines piece types
    - Defines specyfic chess logic elements (castling rights)
	- NOTE: We use int32_t and uint32_t because C++ arithmetic always tends to cast int8_t and int16_t to 32-bit integers
*/


// ---------
// Bitboards
// ---------

using Bitboard = uint64_t;


// -------------------
// Board ranks & files
// -------------------

enum Rank : uint32_t {
    RANK_1 = 0, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8,

    NULL_RANK = 8,
    RANK_RANGE = 8
};

constexpr inline Rank flip(Rank r)
{
	return Rank(r ^ 7);
}

inline std::ostream& operator<<(std::ostream& os, Rank rank)
{
    os << int(rank + 1);

    return os;
}

enum File : uint32_t {
    FILE_A = 0, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H,

    NULL_FILE = 8,
    FILE_RANGE = 8
};

constexpr inline File flip(File f)
{
	return File(f ^ 7);
}

inline std::ostream& operator<<(std::ostream& os, File file)
{
    os << char('a' + file);

    return os;
}


// -------------
// Board squares
// -------------

enum Square : uint32_t {
    // We start from 0 for LSB and MSB algorithms compatibility
	SQ_A1 = 0, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
	SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
	SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
	SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
	SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
	SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
	SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
	SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,

    NULL_SQUARE = 64,
    SQUARE_RANGE = 64,
};

constexpr inline File file_of(Square s)
{
	return File(s & 0x7);
}

constexpr inline Rank rank_of(Square s)
{
	return Rank(s >> 3);
}

constexpr inline Square make_square(Rank rank, File file)
{
    return Square((rank << 3) | file);
}

constexpr inline Bitboard square_to_bb(Square s)
{
	return 1ULL << s;
}

// Horizontal flip - flips rank, for example RANK_1 -> RANK_8, RANK_2 -> RANK_7, etc.
constexpr inline Square flip_rank(Square sq)
{
	return Square(sq ^ 56);
}

// Vertical flip - flips files, for example FILE_A -> FILE_H, FILE_B -> FILE_G, etc.
constexpr inline Square flip_file(Square sq)
{
	return Square(sq ^ 7);
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

inline std::ostream& operator<<(std::ostream& os, Square sq)
{
	os << file_of(sq) << rank_of(sq);

	return os;
}


// -------------
// Square colors
// -------------

enum SquareColor : uint32_t {
	DARK_SQUARE = 0, LIGHT_SQUARE,

	SQUARE_COLOR_RANGE = 2
};

constexpr inline SquareColor operator~(SquareColor color)
{
	return SquareColor(color ^ 0x1);
}

constexpr inline SquareColor color_of(Square sq)
{
	return SquareColor((uint32_t(sq) ^ uint32_t(rank_of(sq))) & 0x1);
}

constexpr inline bool operator==(Square sq, SquareColor color)
{
	return color_of(sq) == color;
}

constexpr inline bool operator==(SquareColor color, Square sq)
{
	return sq == color;
}


// ----------
// Directions
// ----------

// Direction is represented as classical geographic direction, which covers directions of all file, rank, and diagonal moves
// Each direction is encoded with appropriate shift that direction implies to given square
enum Direction : int32_t {
	NORTH = 8,
	SOUTH = -8,
	EAST = 1,
	WEST = -1,

	NORTH_EAST = 9,
	NORTH_WEST = 7,
	SOUTH_EAST = -7,
	SOUTH_WEST = -9
};

// Calculates opposite direction
constexpr inline Direction operator~(Direction dir)
{
	return Direction(-dir);
}

// Check whether derivative direction is exactly primary direction dir or is derived from dir
template <Direction dir>
constexpr inline bool derived(Direction derivative)
{
	return dir == NORTH ? derivative > 1 :
		   dir == SOUTH ? derivative < -1 :
		   dir == EAST ? (derivative & 0x3) == 0x1 :
		   dir == WEST ? (derivative & 0x3) == 0x3 : dir == derivative;
}

// NOTE: needs to be used with caution for WEST and EAST shifts, since doing WEST shift on A-file square results in H-file square
constexpr inline Square operator+(Square sq, Direction dir)
{
	// We are happy to allow overflows here
	uint32_t target = uint32_t(sq) + int32_t(dir);

	return target >= SQUARE_RANGE ? NULL_SQUARE : Square(target);
}

// NOTE: needs to be used with caution for WEST and EAST shifts, since doing EAST shift on A-file square results in H-file square
constexpr inline Square operator-(Square sq, Direction dir)
{
	// We are happy to allow overflows here
	uint32_t target = uint32_t(sq) - int32_t(dir);

	return target >= SQUARE_RANGE ? NULL_SQUARE : Square(target);
}


// -------------
// Playing sides
// -------------

enum Color : uint32_t {
	WHITE = 0, BLACK,

	COLOR_RANGE = 2
};

constexpr inline Color operator~(Color side)
{
	return Color(side ^ 0x1);
}

inline std::ostream& operator<<(std::ostream& os, Color color)
{
	os << (color == WHITE ? "W" : "B");

	return os;
}


// -----------
// Piece types
// -----------

enum PieceType : uint32_t {
	PAWN = 1, KNIGHT, BISHOP, ROOK, QUEEN, KING,

	ALL_PIECES,

	NULL_PIECE_TYPE = 0,
	PIECE_TYPE_RANGE = 8
};

enum Piece : uint32_t {
	NO_PIECE = 0,

	W_PAWN = 1, 
	W_KNIGHT, 
	W_BISHOP, 
	W_ROOK, 
	W_QUEEN, 
	W_KING,

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
	return Color(bool(piece & BLACK_PIECE));
}

constexpr inline PieceType type_of(Piece piece)
{
	return PieceType(piece & 0x7);
}

constexpr inline Piece make_piece(Color color, PieceType type)
{
	return Piece(type | (color << 3));
}

inline std::ostream& operator<<(std::ostream& os, Piece piece)
{
	PieceType ptype = type_of(piece);
	char symbol = ptype == PAWN ? 'P' : ptype == KNIGHT ? 'N' : ptype == BISHOP ? 'B' : 
				  ptype == ROOK ? 'R' : ptype == QUEEN ? 'Q' : ptype == KING ? 'K' : ' ';

	os << char((color_of(piece) == WHITE ? symbol : tolower(symbol)));
	
	return os;
}


// ---------------
// Castling rights
// ---------------

// We start enumeration from 1 to make it compatible with CastlingRights representation
enum Castle : uint32_t {
	KINGSIDE_CASTLE = 1,
	QUEENSIDE_CASTLE,

	NULL_CASTLE = 0,
	CASTLE_RANGE = 3,
};

// Castling rights can be represented as 4-bit integer
// - Every bit represents one of two castling options for one of the sides
// - Least significant bit represents kingside castle for white, then queenside castle for white, and then black rights in same order
using CastlingRights = uint32_t;

// Map castle type to castling right 4-bit integer
constexpr inline CastlingRights make_castle_right(Color side, Castle castle)
{
	return CastlingRights(castle << side * 2);
}

// A list of common castling rights:
// - NOTE: all kingside castle rights are on odd bits, and all queenside castle rights are on even bits
constexpr CastlingRights NO_CASTLING = 0;

constexpr CastlingRights WHITE_OO = make_castle_right(WHITE, KINGSIDE_CASTLE);
constexpr CastlingRights WHITE_OOO = make_castle_right(WHITE, QUEENSIDE_CASTLE);
constexpr CastlingRights BLACK_OO = make_castle_right(BLACK, KINGSIDE_CASTLE);
constexpr CastlingRights BLACK_OOO = make_castle_right(BLACK, QUEENSIDE_CASTLE);

constexpr CastlingRights WHITE_BOTH = WHITE_OO | WHITE_OOO;
constexpr CastlingRights BLACK_BOTH = BLACK_OO | BLACK_OOO;
constexpr CastlingRights ALL_RIGHTS = WHITE_BOTH | BLACK_BOTH;