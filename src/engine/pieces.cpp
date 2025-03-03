#include "pieces.h"
#include "randomgen.h"
#include <vector>


namespace Pieces {

    // -----------
	// Ray attacks
	// -----------

    Bitboard file_attacks(Square sq, Bitboard occ)
    {
        return ray_attacks<NORTH>(sq, occ) | ray_attacks<SOUTH>(sq, occ);
    }

    Bitboard rank_attacks(Square sq, Bitboard occ)
    {
        return ray_attacks<EAST>(sq, occ) | ray_attacks<WEST>(sq, occ);
    }

    Bitboard diagonal_attacks(Square sq, Bitboard occ)
    {
        return ray_attacks<NORTH_EAST>(sq, occ) | ray_attacks<SOUTH_WEST>(sq, occ);
    }

    Bitboard antidiagonal_attacks(Square sq, Bitboard occ)
    {
        return ray_attacks<NORTH_WEST>(sq, occ) | ray_attacks<SOUTH_EAST>(sq, occ);
    }


    // -------------------------------------
	// Single piece attacks - by calculation
	// -------------------------------------

    // We put them in separate namespace to avoid name conflicts with lookup versions of those functions
    namespace Calculation {

        // A general definition of piece attack calculation function
        using Calculator = Bitboard (*)(Square, Bitboard);

        Bitboard rook_attacks(Square sq, Bitboard occ)
        {
            return rank_attacks(sq, occ) | file_attacks(sq, occ);
        }

        Bitboard bishop_attacks(Square sq, Bitboard occ)
        {
            return diagonal_attacks(sq, occ) | antidiagonal_attacks(sq, occ);
        }

        Bitboard queen_attacks(Square sq, Bitboard occ)
        {
            return rook_attacks(sq, occ) | bishop_attacks(sq, occ);
        }
        
    }


    // ------------------------------------------------
	// Single piece attacks - by lookup - lookup tables
	// ------------------------------------------------

    Bitboard PawnAttacks[COLOR_RANGE][SQUARE_RANGE];
	Bitboard PseudoAttacks[PIECE_TYPE_RANGE][SQUARE_RANGE];

    // Magic helpers
    // - Those tables serve as database for all precalculated sliding piece attacks
    // - All magics have pointers to appropriate part of one of those tables
    Bitboard RookTable[102400] = {};
	Bitboard BishopTable[5248] = {};

    // Main magic tables
	Magic RookMagics[SQUARE_RANGE];
	Magic BishopMagics[SQUARE_RANGE];


    // -------------
	// Initilization
	// -------------

    // Initialize all the magic tables
    // - Takes attack calculator function (func) which allows to generalize for different piece types
    // - Uses randomized search approach to find good magic numbers
    // - Usually takes up to 2 seconds to initialize, depending on the seed
    void initialize_magics(Magic* magics, Bitboard* table, Calculation::Calculator attack_calc)
    {
        // Hyperparameters
        constexpr int MAX_ATTACK_TABLE_SIZE = 4096;
		constexpr int RANDOM_SEED = 128;

        // Helper tables
        // - We use std::vector as a safe and simple way of allocating data on heap instead of stack
        std::vector<Bitboard> occupancies(MAX_ATTACK_TABLE_SIZE, 0),
                              attacks(MAX_ATTACK_TABLE_SIZE, 0),
                              mhelper(MAX_ATTACK_TABLE_SIZE, 0);

        int size = 0;

        // Magic values must be initilized for all possible placement of piece
        for (int sq = 0; sq < SQUARE_RANGE; sq++) {
            // Since attack maps do not change if we put any blockers on edge files or ranks, we can extract them
            // to make index smaller
            Bitboard edges = ((Chessboard::RANK_1 | Chessboard::RANK_8) & ~Chessboard::rank(rank_of(Square(sq)))) |
							 ((Chessboard::FILE_A | Chessboard::FILE_H) & ~Chessboard::file(file_of(Square(sq))));
			Bitboard mask = attack_calc(Square(sq), 0) & ~edges;

            Magic& m = magics[sq];
			m.mask = mask;
			m.shift = 64 - Bitboards::popcount(m.mask);
			m.attacks = sq == SQ_A1 ? table : magics[sq - 1].attacks + size;    // Shift the pointer to appropriate location

            size = 0;

            // Now calculate attacks for every possible occupancy that affects attack map
            Bitboard bb = 0;
			do {
				occupancies[size] = bb;
				attacks[size] = attack_calc(Square(sq), bb);
				bb = (bb - mask) & mask;
				size++;
			} while (bb != 0);

            Random::MagicsGenerator gen(RANDOM_SEED);

            // I will leave this code without explanation because it was so long ago the last time I touched it
            // that I don't even remember how does this shit work :)
			uint64_t magic;
			for (int i = 0; i < size; ) {
				for (magic = 0; Bitboards::popcount((magic * mask) >> 56) < 6; magic = gen.sparse_random()) 
					continue;
				m.magic = magic;
				for (i = 0; i < size; i++) {
					int id = m.index(occupancies[i]);
					if (mhelper[id] != magic) {
						m.attacks[id] = attacks[i];
						mhelper[id] = magic;
					}
					else if (m.attacks[id] != attacks[i]) break;
				}
			}
        }
    }

    void initialize_attack_tables()
    {
        // Magics initialization
        initialize_magics(RookMagics, RookTable, Calculation::rook_attacks);
		initialize_magics(BishopMagics, BishopTable, Calculation::bishop_attacks);

		for (int sq = SQ_A1; sq <= SQ_H8; ++sq) {
			Bitboard squareBB = square_to_bb(Square(sq));
			PawnAttacks[WHITE][sq] = pawn_attacks<WHITE>(squareBB);
			PawnAttacks[BLACK][sq] = pawn_attacks<BLACK>(squareBB);
			PseudoAttacks[KNIGHT][sq] = knight_attacks(squareBB);
			PseudoAttacks[KING][sq] = king_attacks(squareBB);
			PseudoAttacks[BISHOP][sq] = Calculation::bishop_attacks(Square(sq), 0);
			PseudoAttacks[ROOK][sq] = Calculation::rook_attacks(Square(sq), 0);
			PseudoAttacks[QUEEN][sq] = Calculation::queen_attacks(Square(sq), 0);
		}
    }

}