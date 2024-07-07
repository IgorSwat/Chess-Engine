#include "pieces.h"
#include "randomGens.h"
#include <algorithm>
#include <iostream>

namespace Pieces {

	// ---------------------------
	// Precalculated attack tables
	// ---------------------------

	Bitboard RAY_ATTACKS[SQUARE_RANGE][DIRECTION_RANGE];

	Bitboard PAWN_ATTACKS[COLOR_RANGE][SQUARE_RANGE];
	Bitboard PIECE_ATTACKS[PIECE_TYPE_RANGE][SQUARE_RANGE];
	Magic ROOK_MAGICS[SQUARE_RANGE];
	Magic BISHOP_MAGICS[SQUARE_RANGE];


	// ---------------------------------------------
	// Ray attacks by calculation - Classical Method
	// ---------------------------------------------

	void initRayAttacks()
	{
		for (int sq = SQ_A1; sq <= SQ_H8; ++sq) {
			for (int dir = WEST; dir <= NORTH_WEST; ++dir) {
				Bitboard attacks = 0;
				Bitboard squareBB = square_to_bb(Square(sq));
				while (squareBB != 0) {
					squareBB = Bitboards::shift_d(squareBB, Direction(dir));
					attacks |= squareBB;
				}
				RAY_ATTACKS[sq][dir] = attacks;
			}
		}
	}

	template <Direction dir>
	Bitboard positiveRayAttacks(Square sq, Bitboard occ)
	{
		Bitboard attacks = RAY_ATTACKS[sq][dir];
		Bitboard blockers = attacks & occ;
		unsigned long blockerSquare = 0;
		_BitScanForward64(&blockerSquare, blockers | 0x8000000000000000);	// To prevent bitscan on empty bitboard
		return attacks ^ RAY_ATTACKS[blockerSquare][dir];
	}

	template <Direction dir>
	Bitboard negativeRayAttacks(Square sq, Bitboard occ)
	{
		Bitboard attacks = RAY_ATTACKS[sq][dir];
		Bitboard blockers = attacks & occ;
		unsigned long blockerSquare = 0;
		_BitScanReverse64(&blockerSquare, blockers | 0x1);	// To prevent bitscan on empty bitboard
		return attacks ^ RAY_ATTACKS[blockerSquare][dir];
	}

	Bitboard diagonalAttacks(Square sq, Bitboard occ)
	{
		return positiveRayAttacks<NORTH_EAST>(sq, occ) | negativeRayAttacks<SOUTH_WEST>(sq, occ);
	}

	Bitboard antidiagonalAttacks(Square sq, Bitboard occ)
	{
		return positiveRayAttacks<NORTH_WEST>(sq, occ) | negativeRayAttacks<SOUTH_EAST>(sq, occ);
	}

	Bitboard rankAttacks(Square sq, Bitboard occ)
	{
		return positiveRayAttacks<EAST>(sq, occ) | negativeRayAttacks<WEST>(sq, occ);
	}

	Bitboard fileAttacks(Square sq, Bitboard occ)
	{
		return positiveRayAttacks<NORTH>(sq, occ) | negativeRayAttacks<SOUTH>(sq, occ);
	}


	// -------------------------------------------------------
	// Sliding piece attacks by calculation - Classical Method
	// -------------------------------------------------------

	Bitboard rookAttacksCalc(Square sq, Bitboard occ)
	{
		return rankAttacks(sq, occ) | fileAttacks(sq, occ);
	}

	Bitboard bishopAttacksCalc(Square sq, Bitboard occ)
	{
		return diagonalAttacks(sq, occ) | antidiagonalAttacks(sq, occ);
	}

	Bitboard queenAttacksCalc(Square sq, Bitboard occ)
	{
		return rookAttacksCalc(sq, occ) | bishopAttacksCalc(sq, occ);
	}


	// -------------------
	// Magics initializers
	// -------------------

	Bitboard rookTable[102400] = {};
	Bitboard bishopTable[5248] = {};

	void initMagics(Magic* magics, Bitboard* table, AttacksCalculator attacksCalc)
	{
		constexpr int MAX_ATTACK_TABLE_SIZE = 4096;
		constexpr int RANDOM_SEED = 128;

		Bitboard* occupancies = new Bitboard[MAX_ATTACK_TABLE_SIZE]{};
		Bitboard* attacks = new Bitboard[MAX_ATTACK_TABLE_SIZE]{};
		uint64_t* magicsHelper = new uint64_t[MAX_ATTACK_TABLE_SIZE]{};
		int size = 0;

		for (int sq = SQ_A1; sq <= SQ_H8; ++sq) {
			Bitboard edges = ((Board::RANK_1 | Board::RANK_8) & ~Board::Ranks[rank_of(Square(sq))]) |
							 ((Board::FILE_A | Board::FILE_H) & ~Board::Files[file_of(Square(sq))]);
			Bitboard mask = attacksCalc(Square(sq), 0) & ~edges;
			Magic& m = magics[sq];
			m.mask = mask;
			m.shift = 64 - Bitboards::popcount(m.mask);
			m.attacks = sq == SQ_A1 ? table : magics[sq - 1].attacks + size;

			size = 0;
			Bitboard bb = 0;
			do {
				occupancies[size] = bb;
				attacks[size] = attacksCalc(Square(sq), bb);
				bb = (bb - mask) & mask;
				size++;
			} while (bb != 0);

			MagicsGenerator gen(RANDOM_SEED);
			uint64_t magic;
			for (int i = 0; i < size; ) {
				for (magic = 0; Bitboards::popcount((magic * mask) >> 56) < 6; magic = gen.sparseRandom()) 
					continue;
				m.magic = magic;
				for (i = 0; i < size; i++) {
					int id = m.index(occupancies[i]);
					if (magicsHelper[id] != magic) {
						m.attacks[id] = attacks[i];
						magicsHelper[id] = magic;
					}
					else if (m.attacks[id] != attacks[i]) break;
				}
			}
		}

		delete[] occupancies;
		delete[] attacks;
		delete[] magicsHelper;
	}

	void initAttackTables()
	{
		initRayAttacks();
		initMagics(ROOK_MAGICS, rookTable, rookAttacksCalc);
		initMagics(BISHOP_MAGICS, bishopTable, bishopAttacksCalc);
		for (int sq = SQ_A1; sq <= SQ_H8; ++sq) {
			Bitboard squareBB = square_to_bb(Square(sq));
			PAWN_ATTACKS[WHITE][sq] = pawn_attacks<WHITE>(squareBB);
			PAWN_ATTACKS[BLACK][sq] = pawn_attacks<BLACK>(squareBB);
			PIECE_ATTACKS[KNIGHT][sq] = knight_attacks(squareBB);
			PIECE_ATTACKS[KING][sq] = king_attacks(squareBB);
			PIECE_ATTACKS[BISHOP][sq] = bishopAttacksCalc(Square(sq), 0);
			PIECE_ATTACKS[ROOK][sq] = rookAttacksCalc(Square(sq), 0);
			PIECE_ATTACKS[QUEEN][sq] = queenAttacksCalc(Square(sq), 0);
		}
	}


	// ---------------
	// Other functions
	// ---------------

	Bitboard piece_attacks_d(PieceType type, Square sq, Bitboard occ)
	{
		return type == KNIGHT ? piece_attacks_s<KNIGHT>(sq) :
			type == BISHOP ? piece_attacks_s<BISHOP>(sq, occ) :
			type == ROOK ? piece_attacks_s<ROOK>(sq, occ) :
			type == QUEEN ? piece_attacks_s<QUEEN>(sq, occ) : piece_attacks_s<KING>(sq);
	}
}