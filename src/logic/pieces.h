#pragma once

#include "bitboards.h"

// Piece-specyfic operations and bitboards
namespace Pieces {
	extern Bitboard PAWN_ATTACKS[COLOR_RANGE][SQUARE_RANGE];
	extern Bitboard PIECE_ATTACKS[PIECE_TYPE_RANGE][SQUARE_RANGE];	// Contains legal attacks for king / knight & pseudo attacks for sliding pieces

	constexpr Bitboard KING_CASTLING_PATHS[CASTLING_RIGHTS_RANGE] = {
		0,0x0000000000000060, 0x000000000000000e, 0, 0x6000000000000000, 0, 0, 0,
		0x0e00000000000000, 0, 0, 0, 0, 0, 0, 0
	};

	struct Magic
	{
		Bitboard* attacks;
		Bitboard mask;
		uint64_t magic;
		int shift;

		int index(Bitboard bb) const
		{
			return int(((bb & mask) * magic) >> shift);
		}
	};

	using AttacksCalculator = Bitboard(*)(Square, Bitboard);

	extern Magic ROOK_MAGICS[SQUARE_RANGE];
	extern Magic BISHOP_MAGICS[SQUARE_RANGE];

	void initAttackTables();

	template <Color side>
	inline Bitboard pawnAttacks(Bitboard pawnsBB)
	{
		Bitboard l = (pawnsBB & NOT_FILE_A) >> 1;
		Bitboard r = (pawnsBB & NOT_FILE_H) << 1;
		Bitboard h = l | r;
		return side == WHITE ? h << 8 : h >> 8;
	}

	// Returns squares attacked by two pawns of given color
	template <Color side>
	inline Bitboard doublePawnAttacks(Bitboard pawnsBB)
	{
		constexpr Direction forwardLeft = (side == WHITE ? NORTH_WEST : SOUTH_WEST);
		constexpr Direction forwardRight = (side == WHITE ? NORTH_EAST : SOUTH_EAST);
		return Bitboards::shift<forwardLeft>(pawnsBB) & Bitboards::shift<forwardRight>(pawnsBB);
	}

	inline Bitboard knightAttacks(Bitboard knightsBB)
	{
		Bitboard l1 = (knightsBB & NOT_FILE_A) >> 1;
		Bitboard l2 = (knightsBB & NOT_FILE_AB) >> 2;
		Bitboard r1 = (knightsBB & NOT_FILE_H) << 1;
		Bitboard r2 = (knightsBB & NOT_FILE_GH) << 2;
		Bitboard h1 = l1 | r1;
		Bitboard h2 = l2 | r2;
		return (h1 << 16) | (h1 >> 16) | (h2 << 8) | (h2 >> 8);
	}

	inline Bitboard kingAttacks(Bitboard kingBB)
	{
		Bitboard attacks = ((kingBB & NOT_FILE_H) << 1) | ((kingBB & NOT_FILE_A) >> 1);
		kingBB |= attacks;
		attacks |= (kingBB << 8) | (kingBB >> 8);
		return attacks;
	}

	inline Bitboard pawnAttacks(Color side, Square sq)
	{
		return PAWN_ATTACKS[side][sq];
	}

	template <PieceType type>
	inline Bitboard pieceAttacks(Square sq, Bitboard occ = 0)
	{
		return PIECE_ATTACKS[type][sq];
	}

	template <>
	inline Bitboard pieceAttacks<BISHOP>(Square sq, Bitboard occ)
	{
		Magic& m = BISHOP_MAGICS[sq];
		return m.attacks[m.index(occ)];
	}

	template <>
	inline Bitboard pieceAttacks<ROOK>(Square sq, Bitboard occ)
	{
		Magic& m = ROOK_MAGICS[sq];
		return m.attacks[m.index(occ)];
	}

	template <>
	inline Bitboard pieceAttacks<QUEEN>(Square sq, Bitboard occ)
	{
		return pieceAttacks<BISHOP>(sq, occ) | pieceAttacks<ROOK>(sq, occ);
	}

	inline Bitboard pieceAttacks(PieceType type, Square sq, Bitboard occ)
	{
		return type == KNIGHT ? pieceAttacks<KNIGHT>(sq) :
			type == BISHOP ? pieceAttacks<BISHOP>(sq, occ) :
			type == ROOK ? pieceAttacks<ROOK>(sq, occ) :
			type == QUEEN ? pieceAttacks<QUEEN>(sq, occ) : pieceAttacks<KING>(sq);
	}

	inline Bitboard pseudoAttacks(PieceType piece, Square sq)
	{
		return PIECE_ATTACKS[piece][sq];
	}

	inline Bitboard xRayRookAttacks(Square sq, Bitboard occ, Bitboard blockers)	// By default blockers = own pieces
	{
		Bitboard attacks = pieceAttacks<ROOK>(sq, occ);
		blockers &= attacks;
		return attacks ^ pieceAttacks<ROOK>(sq, occ ^ blockers);
	}

	inline Bitboard xRayBishopAttacks(Square sq, Bitboard occ, Bitboard blockers)
	{
		Bitboard attacks = pieceAttacks<BISHOP>(sq, occ);
		blockers &= attacks;
		return attacks ^ pieceAttacks<BISHOP>(sq, occ ^ blockers);
	}

	inline Bitboard xRayQueenAttacks(Square sq, Bitboard occ, Bitboard blockers)
	{
		return xRayBishopAttacks(sq, occ, blockers) | xRayRookAttacks(sq, occ, blockers);
	}

	inline bool inPieceDistance(PieceType type, Square sq1, Square sq2)
	{
		return pseudoAttacks(type, sq1) & sq2;
	}

	constexpr inline int kingDistance(Square sq1, Square sq2)
	{
		return SQUARE_DISTANCE[sq1][sq2];
	}

	constexpr inline Bitboard castlingPath(CastleType castle)
	{
		return KING_CASTLING_PATHS[castle];
	}
}