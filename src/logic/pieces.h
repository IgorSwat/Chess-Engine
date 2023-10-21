#pragma once

#include "bitboards.h"

// Piece-specyfic operations and bitboards
namespace Pieces {
	extern Bitboard PAWN_ATTACKS[COLOR_RANGE][SQUARE_RANGE];
	extern Bitboard PIECE_ATTACKS[PIECE_TYPE_RANGE][SQUARE_RANGE];	// Contains legal attacks for king / knight & pseudo attacks for sliding pieces

	template <Color side>
	inline Bitboard pawnAttacks(Bitboard pawnsBB)
	{
		Bitboard l = (pawnsBB & NOT_FILE_A) >> 1;
		Bitboard r = (pawnsBB & NOT_FILE_H) << 1;
		Bitboard h = l | r;
		return side == WHITE ? h >> 8 : h << 8;
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


	Bitboard diagonalAttacks(Bitboard occ, Square sq);
	Bitboard antidiagonalAttacks(Bitboard occ, Square sq);
	Bitboard rowAttacks(Bitboard occ, Square sq);
	Bitboard fileAttacks(Bitboard occ, Square sq);
	Bitboard rookAttacksCalc(Bitboard occ, Square sq);
	Bitboard bishopAttacksCalc(Bitboard occ, Square sq);
	Bitboard queenAttacksCalc(Bitboard occ, Square sq);


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

	using AttacksCalculator = Bitboard(*)(Bitboard, Square);

	extern Magic ROOK_MAGICS[SQUARE_RANGE];
	extern Magic BISHOP_MAGICS[SQUARE_RANGE];

	void initAttackTables();


	inline Bitboard bishopAttacks(Bitboard occ, Square sq)
	{
		Magic& m = BISHOP_MAGICS[sq];
		return m.attacks[m.index(occ)];
	}

	inline Bitboard rookAttacks(Bitboard occ, Square sq)
	{
		Magic& m = ROOK_MAGICS[sq];
		return m.attacks[m.index(occ)];
	}

	inline Bitboard queenAttacks(Bitboard occ, Square sq)
	{
		return bishopAttacks(occ, sq) | rookAttacks(occ, sq);
	}

	inline Bitboard pieceAttacks(PieceType piece, Bitboard occ, Square sq)
	{
		return piece == KNIGHT ? PIECE_ATTACKS[KNIGHT][sq] :
			piece == BISHOP ? bishopAttacks(occ, sq) :
			piece == ROOK ? rookAttacks(occ, sq) :
			piece == QUEEN ? queenAttacks(occ, sq) :
			piece == KING ? PIECE_ATTACKS[KING][sq] : 0;
	}
}