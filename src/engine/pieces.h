#pragma once

#include "bitboards.h"
#include "boardspace.h"


/*
    ---------- Pieces ----------

    A library focusing on piece movement and attack properties.
    - Basic ray attack calculation algorithms
    - Single piece attacks by calculation (using ray attacks)
    - Single piece attacks by lookup (using magic bitboards)
    - Multiple piece attacks (for pawn, knight and king) - aggregative calculation
*/

namespace Pieces {

    // -------------
	// Initilization
	// -------------

    void initialize_attack_tables();


    // -----------
	// Ray attacks
	// -----------

    // Ray attacks are parts of bishop, rook and queen attack maps
    // - We can calculate ray attacks on empty board using fill algorithm from Bitboards library
    // - To calculate ray attack with given occupancy, we can follow the path calculation method from boardspace.cpp

    // Ray attacks (in one direction) with respect to given board occupancy
    template <Direction dir>
    Bitboard ray_attacks(Square sq, Bitboard occ = 0)
    {
        Bitboard att = Bitboards::fill<dir>(square_to_bb(sq)) ^ sq;   // Do not count attacking the starting square
        Bitboard blockers = att & occ;

        return !blockers ? att :
               dir > 0   ? att & Bitboards::fill<~dir>(square_to_bb(Bitboards::lsb(blockers))) :
                           att & Bitboards::fill<~dir>(square_to_bb(Bitboards::msb(blockers)));
    }

    // Bidirectional attacks
    Bitboard file_attacks(Square sq, Bitboard occ = 0);             // NORTH and SOUTH directions
    Bitboard rank_attacks(Square sq, Bitboard occ = 0);             // WEST and EAST directions
    Bitboard diagonal_attacks(Square sq, Bitboard occ = 0);         // NORTH_EAST and SOUTH_WEST directions
    Bitboard antidiagonal_attacks(Square sq, Bitboard occ = 0);     // NORTH_WEST and SOUTH_EAST directions


    // -------------------------------------
	// Single piece attacks - by calculation
	// -------------------------------------

    // Since using ray attack calculations to calculate sliding piece attacks is inefficient and therefore deprecated,
    // they are not a part of the public API of this module


    // -----------------------------------------
	// Single piece attacks - by lookup - magics
	// -----------------------------------------

    // Magic bitboards is a perfect hashing algorithm that allows for fast lookup instead of calculation of piece attacks

    struct Magic
	{
		Bitboard* attacks;      // Pointer to appropriate part of lookup table
		Bitboard mask;          // Mask = {attacks from sq} - {board edges}
		uint64_t magic;         // A magic number found by randomized search that performs best in given case
		uint32_t shift;         // Shift = 64 - popcount(mask)

        // Lookup table index calculation
		uint32_t index(Bitboard bb) const
		{
			return uint32_t(((bb & mask) * magic) >> shift);
		}
	};


    // -------------------------------------------------
	// Single piece attacks - by lookup - direct attacks
	// -------------------------------------------------

    // Lookup tables
    // - Pseudo attack is attack that ignores occupancy factor (which would be legal on empty board)
    // - NOTE: PieceAttacks contains only pseudo legal moves, which are equivalent of legal moves for knight and king,
    //         but discard occupancy factor for sliding pieces (bishops, rooks and queens)

    extern Bitboard PawnAttacks[COLOR_RANGE][SQUARE_RANGE];
	extern Bitboard PseudoAttacks[PIECE_TYPE_RANGE][SQUARE_RANGE];

	extern Magic RookMagics[SQUARE_RANGE];
	extern Magic BishopMagics[SQUARE_RANGE];

    // Lookup table getters
    // - NOTE: It is recommended to always use getter functions instead of lookup tables if possible

    inline Bitboard pawn_attacks(Color side, Square sq)
    {
        return PawnAttacks[side][sq];
    }

    inline Bitboard pseudo_attacks(PieceType ptype, Square sq)
    {
        return PseudoAttacks[ptype][sq];
    }

    // General static version - for knight & king attacks
    template <PieceType ptype>
    inline Bitboard piece_attacks_s(Square sq, Bitboard occ = 0)
    {
        return PseudoAttacks[ptype][sq];
    }

    // Specialized static version - sliding piece attacks - bishop magics lookup
    template <>
	inline Bitboard piece_attacks_s<BISHOP>(Square sq, Bitboard occ)
	{
		Magic& m = BishopMagics[sq];
		return m.attacks[m.index(occ)];
	}

    // Specialized static version - sliding piece attacks - rook magics lookup
    template <>
	inline Bitboard piece_attacks_s<ROOK>(Square sq, Bitboard occ)
	{
		Magic& m = RookMagics[sq];
		return m.attacks[m.index(occ)];
	}

    // Specialized static version - sliding piece attacks - queen attacks by combining bishop and rook attacks
    template <>
	inline Bitboard piece_attacks_s<QUEEN>(Square sq, Bitboard occ)
	{
		return piece_attacks_s<BISHOP>(sq, occ) | piece_attacks_s<ROOK>(sq, occ);
	}

    // Dynamic version - sliding piece attacks
    // - A little bit slower, but works with runtime parameters
    inline Bitboard piece_attacks_d(PieceType ptype, Square sq, Bitboard occ)
    {
        return ptype == KNIGHT ? PseudoAttacks[KNIGHT][sq] :
               ptype == BISHOP ? piece_attacks_s<BISHOP>(sq, occ) :
               ptype == ROOK ? piece_attacks_s<ROOK>(sq, occ) :
               ptype == QUEEN ? piece_attacks_s<QUEEN>(sq, occ) :
               ptype == KING ? PseudoAttacks[KING][sq] : 0;
    }


    // ------------------------------------------------
	// Single piece attacks - by lookup - x-ray attacks
	// ------------------------------------------------

    // X-ray attacks is a pseudo attack blocked by a friendly piece with the same properties as attacking piece
    // For example, rook attacks are extended to x-ray attacks if it stands behind another rook or queen
    // - X-ray attacks play a key role in SEE (Static Exchange Evaluation) algorithm

    // NOTE: results in undefined behavior for non-sliding pieces (pawn, knight, king)
    template <PieceType ptype>
    inline Bitboard xray_attacks(Square sq, Bitboard occ, Bitboard blockers)
    {
        Bitboard attacks = piece_attacks_s<ptype>(sq, occ);
		blockers &= attacks;

		return attacks ^ piece_attacks_s<ptype>(sq, occ ^ blockers);
    }

    template <>
	inline Bitboard xray_attacks<QUEEN>(Square sq, Bitboard occ, Bitboard blockers)
	{
		return xray_attacks<BISHOP>(sq, occ, blockers) | xray_attacks<ROOK>(sq, occ, blockers);
	}


    // ----------------------
	// Multiple piece attacks
	// ----------------------

    // The following algorithms perform aggregative attack calculation for non sliding pieces (pawns, knights and king)
    // - Aggregative calculation means it can calculate a whole map of attacks of all pieces of given type in one go
    // - Utilizes divide & conquer approach on bitboards

    // Each square can only be attacked by one pawn...
    template <Color side>
    constexpr inline Bitboard pawn_attacks(Bitboard pawns)
    {
        Bitboard ar_squares = (pawns & Board::NOT_FILE_A) >> 1 | (pawns & Board::NOT_FILE_H) << 1;

        return side == WHITE ? Bitboards::shift_s<NORTH>(ar_squares) :
                               Bitboards::shift_s<SOUTH>(ar_squares);
    }

    // or two pawns
    template <Color side>
    constexpr inline Bitboard double_pawn_attacks(Bitboard pawns)
    {
        return side == WHITE ? Bitboards::shift_s<NORTH_WEST>(pawns) & Bitboards::shift_s<NORTH_EAST>(pawns) :
                               Bitboards::shift_s<SOUTH_WEST>(pawns) & Bitboards::shift_s<SOUTH_EAST>(pawns);
    }

    constexpr inline Bitboard knight_attacks(Bitboard knights)
    {
        Bitboard l1 = (knights & Board::NOT_FILE_A) >> 1;
		Bitboard l2 = (knights & Board::NOT_FILE_AB) >> 2;
		Bitboard r1 = (knights & Board::NOT_FILE_H) << 1;
		Bitboard r2 = (knights & Board::NOT_FILE_GH) << 2;

		Bitboard h1 = l1 | r1;
		Bitboard h2 = l2 | r2;

		return (h1 << 16) | (h1 >> 16) | (h2 << 8) | (h2 >> 8);
    }

    constexpr inline Bitboard king_attacks(Bitboard kings)
    {
        Bitboard att = (kings & Board::NOT_FILE_H) << 1 | (kings & Board::NOT_FILE_A) >> 1;
        
        kings |= att;
        att |= (kings << 8) | (kings >> 8);

        return att;
    }
}