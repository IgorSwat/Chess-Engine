#pragma once

#include "bitboards.h"


namespace Pieces {

	// ------
	// Magics
	// ------

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


	// ---------------------------
	// Precalculated attack tables
	// ---------------------------

	extern Bitboard PAWN_ATTACKS[COLOR_RANGE][SQUARE_RANGE];
	extern Bitboard PIECE_ATTACKS[PIECE_TYPE_RANGE][SQUARE_RANGE];	// Contains legal attacks for king / knight & pseudo attacks for sliding pieces

	extern Magic ROOK_MAGICS[SQUARE_RANGE];
	extern Magic BISHOP_MAGICS[SQUARE_RANGE];


	void initAttackTables();


	// --------------------------
	// Dynamic attack calculators
	// --------------------------

	using AttacksCalculator = Bitboard(*)(Square, Bitboard);


	template <Color side>
	inline Bitboard pawn_attacks(Bitboard pawnsBB)
	{
		Bitboard l = (pawnsBB & Board::NOT_FILE_A) >> 1;
		Bitboard r = (pawnsBB & Board::NOT_FILE_H) << 1;
		Bitboard h = l | r;
		return side == WHITE ? h << 8 : h >> 8;
	}

	// Returns squares attacked by two pawns of given color
	template <Color side>
	inline Bitboard pawn_attacks_x2(Bitboard pawnsBB)
	{
		constexpr Direction forwardLeft = (side == WHITE ? NORTH_WEST : SOUTH_WEST);
		constexpr Direction forwardRight = (side == WHITE ? NORTH_EAST : SOUTH_EAST);
		return Bitboards::shift_s<forwardLeft>(pawnsBB) & Bitboards::shift_s<forwardRight>(pawnsBB);
	}

	inline Bitboard knight_attacks(Bitboard knightsBB)
	{
		Bitboard l1 = (knightsBB & Board::NOT_FILE_A) >> 1;
		Bitboard l2 = (knightsBB & Board::NOT_FILE_AB) >> 2;
		Bitboard r1 = (knightsBB & Board::NOT_FILE_H) << 1;
		Bitboard r2 = (knightsBB & Board::NOT_FILE_GH) << 2;
		Bitboard h1 = l1 | r1;
		Bitboard h2 = l2 | r2;
		return (h1 << 16) | (h1 >> 16) | (h2 << 8) | (h2 >> 8);
	}

	inline Bitboard king_attacks(Bitboard kingBB)
	{
		Bitboard attacks = ((kingBB & Board::NOT_FILE_H) << 1) | ((kingBB & Board::NOT_FILE_A) >> 1);
		kingBB |= attacks;
		attacks |= (kingBB << 8) | (kingBB >> 8);
		return attacks;
	}


	// ---------------------------
	// Static attack table lookups
	// ---------------------------

	inline Bitboard pawn_attacks(Color side, Square sq)
	{
		return PAWN_ATTACKS[side][sq];
	}

	template <PieceType type>
	inline Bitboard piece_attacks_s(Square sq, Bitboard occ = 0)
	{
		return PIECE_ATTACKS[type][sq];
	}

	template <>
	inline Bitboard piece_attacks_s<BISHOP>(Square sq, Bitboard occ)
	{
		Magic& m = BISHOP_MAGICS[sq];
		return m.attacks[m.index(occ)];
	}

	template <>
	inline Bitboard piece_attacks_s<ROOK>(Square sq, Bitboard occ)
	{
		Magic& m = ROOK_MAGICS[sq];
		return m.attacks[m.index(occ)];
	}

	template <>
	inline Bitboard piece_attacks_s<QUEEN>(Square sq, Bitboard occ)
	{
		return piece_attacks_s<BISHOP>(sq, occ) | piece_attacks_s<ROOK>(sq, occ);
	}

	Bitboard piece_attacks_d(PieceType type, Square sq, Bitboard occ);

	inline Bitboard pseudo_attacks(PieceType piece, Square sq)
	{
		return PIECE_ATTACKS[piece][sq];
	}


	// -------------
	// X-Ray attacks
	// -------------

	template <PieceType type>
	inline Bitboard xray_attacks(Square sq, Bitboard occ, Bitboard blockers)
	{
		Bitboard attacks = piece_attacks_s<type>(sq, occ);
		blockers &= attacks;
		return attacks ^ piece_attacks_s<type>(sq, occ ^ blockers);
	}

	template <>
	inline Bitboard xray_attacks<QUEEN>(Square sq, Bitboard occ, Bitboard blockers)
	{
		return xray_attacks<BISHOP>(sq, occ, blockers) | xray_attacks<ROOK>(sq, occ, blockers);
	}


	// -------------------------
	// Other piece-attack issues
	// -------------------------

	constexpr inline Bitboard castle_path(CastleType castle)
	{
		return castle == WHITE_OO ? 0x0000000000000060 :
			castle == WHITE_OOO ? 0x000000000000000e :
			castle == BLACK_OO ? 0x6000000000000000 :
			castle == BLACK_OOO ? 0x0e00000000000000 : 0;
	}

}