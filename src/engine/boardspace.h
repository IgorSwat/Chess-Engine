#pragma once

#include "bitboards.h"
#include "types.h"
#include <algorithm>
#include <cstdlib>

/*
    ---------- Board space ----------

    Board geometry library.
    - Contains bitboard representations of simple board elements, such as files, ranks, diagonals and others
    - Contains pre-calculated lookup tables and functions for more complex board geometry aspects, such as paths and rectangles
    - Requires initialization (initialize_board_space) before use to work properly
*/

namespace Chessboard {

    // -------------------------------
	// Board geometry - initialization
	// -------------------------------

    void initialize_board_space();


    // -------------------------------
	// Board geometry - simple - files
	// -------------------------------

    constexpr Bitboard FILE_A = 0x0101010101010101;
	constexpr Bitboard FILE_B = FILE_A << 1;
	constexpr Bitboard FILE_C = FILE_A << 2;
	constexpr Bitboard FILE_D = FILE_A << 3;
	constexpr Bitboard FILE_E = FILE_A << 4;
	constexpr Bitboard FILE_F = FILE_A << 5;
	constexpr Bitboard FILE_G = FILE_A << 6;
	constexpr Bitboard FILE_H = FILE_A << 7;

    constexpr Bitboard CENTRAL_FILES = FILE_C | FILE_D | FILE_E | FILE_F;
	constexpr Bitboard BG_FILES = FILE_B | FILE_G;
	constexpr Bitboard EDGE_FILES = FILE_A | FILE_H;
    
	constexpr Bitboard NOT_FILE_A = ~FILE_A;
	constexpr Bitboard NOT_FILE_H = ~FILE_H;
	constexpr Bitboard NOT_FILE_AB = ~(FILE_A | FILE_B);
	constexpr Bitboard NOT_FILE_GH = ~(FILE_G | FILE_H);
	constexpr Bitboard NOT_EDGE_FILES = ~EDGE_FILES;

    // Returns a bitboard representation for any given file
    constexpr inline Bitboard file(File f)
    {
        return FILE_A << f;
    }

    // Returns a bitboard containing both (or just one in case of A or H files) files adjacent to the given file
    constexpr inline Bitboard adjacent_files(File f)
    {
        return Bitboards::shift_s<WEST>(file(f)) | Bitboards::shift_s<EAST>(file(f));
    }

    // Returns bitboard of squares adjacent to given square in the same file
    // - | |x| |
	//	 | |s| |
	//	 | |x| |
    constexpr inline Bitboard adjacent_file_squares(Square sq)
    {
        return Bitboards::shift_s<NORTH>(square_to_bb(sq)) | Bitboards::shift_s<SOUTH>(square_to_bb(sq));
    }


    // -------------------------------
	// Board geometry - simple - ranks
	// -------------------------------

    constexpr Bitboard RANK_1 = 0xff;
	constexpr Bitboard RANK_2 = RANK_1 << (8 * 1);
	constexpr Bitboard RANK_3 = RANK_1 << (8 * 2);
	constexpr Bitboard RANK_4 = RANK_1 << (8 * 3);
	constexpr Bitboard RANK_5 = RANK_1 << (8 * 4);
	constexpr Bitboard RANK_6 = RANK_1 << (8 * 5);
	constexpr Bitboard RANK_7 = RANK_1 << (8 * 6);
	constexpr Bitboard RANK_8 = RANK_1 << (8 * 7);

    constexpr Bitboard CENTRAL_RANKS = RANK_3 | RANK_4 | RANK_5 | RANK_6;
	constexpr Bitboard NOT_RANK_1 = ~RANK_1;
	constexpr Bitboard NOT_RANK_8 = ~RANK_8;
	constexpr Bitboard NOT_RANK_12 = ~(RANK_1 & RANK_2);
	constexpr Bitboard NOT_RANK_78 = ~(RANK_7 & RANK_8);

    // Returns a bitboard representation for any given rank
    constexpr inline Bitboard rank(Rank r)
    {
        return RANK_1 << (8 * r);
    }

    // Returns a bitboard containing both (or just one in case of first or eight rank) ranks adjacent to the given rank
    constexpr inline Bitboard adjacent_ranks(Rank r)
    {
        return Bitboards::shift_s<NORTH>(rank(r)) | Bitboards::shift_s<SOUTH>(rank(r));
    }

    // Returns bitboard of squares adjacent to given square in the same rank
    // - | | | |
	//	 |x|s|x|
	//	 | | | |
    constexpr inline Bitboard adjacent_rank_squares(Square sq)
    {
        // WARNING: In original version, it was returning 0 for NULL_SQUARE
        return Bitboards::shift_s<WEST>(square_to_bb(sq)) | Bitboards::shift_s<EAST>(square_to_bb(sq));
    }


    // -------------------------------
	// Board geometry - simple - other
	// -------------------------------

    constexpr Bitboard DIAG_A1H8 = 0x8040201008040201;
	constexpr Bitboard DIAG_A8H1 = 0x0102040810204080;

    constexpr Bitboard DARK_SQUARES = 0xaa55aa55aa55aa55;
	constexpr Bitboard LIGHT_SQUARES = 0x55aa55aa55aa55aa;

    constexpr Bitboard QUEENSIDE = FILE_A | FILE_B | FILE_C | FILE_D;
	constexpr Bitboard KINGSIDE = FILE_E | FILE_F | FILE_G | FILE_H;

	constexpr Bitboard BOARD = 0xffffffffffffffff;

    // Calculates the distance between two squares using Chebyshev metric
    // - Equivalent to number of king moves required to get from sq1 to sq2 on empty board
    constexpr inline uint32_t distance(Square sq1, Square sq2)
    {
        return std::min(std::abs(int(rank_of(sq2)) - int(rank_of(sq1))), 
                        std::abs(int(file_of(sq2)) - int(file_of(sq1))));
    }

    // --------------------------------
	// Board geometry - complex - lines
	// --------------------------------

    // Predefined lookup table for paths
    // - We define path between squares as a line consitsting of both ends and every square in between
    //   If one square cannot be reached from another by vertical, horizontal or diagonal move, then path from one to the other is 0
    extern Bitboard Paths[SQUARE_RANGE][SQUARE_RANGE];

    // Predefined lookup table for lines
    // - Similar to Paths table, but lines cover the whole file, rank or diagonal that connects two aligned squares
    extern Bitboard Lines[SQUARE_RANGE][SQUARE_RANGE];

    // Two squares (sq1, sq2) are aligned, if path between them satisfies the definition above (non-zero value)
    inline bool aligned(Square sq1, Square sq2)
    {
        return Lines[sq1][sq2];
    }

    // Three squares (sq1, sq2, sq3) are aligned, if common line between two of them exists, and also contains the third one
    inline bool aligned(Square sq1, Square sq2, Square sq3)
    {
        return Lines[sq1][sq3] & sq2;
    }

    // Here we assume, that specifically mid is the "in-between" square
    // - NOTE: returns true if either 'from < mid < to' or 'to < mid < from'
    inline bool aligned_in_order(Square from, Square mid, Square to)
    {
        return Paths[from][to] & mid;
    }


    // --------------------------------
	// Board geometry - complex - boxes
	// --------------------------------

    // A box defined by two squares (sq1, sq2) is a rectangle with opposite corners located exactly in sq1 and sq2 squares
    extern Bitboard Boxes[SQUARE_RANGE][SQUARE_RANGE];


    // --------------------------------
	// Board geometry - complex - spans
	// --------------------------------

    // A span is an area consisting of 3 files, ranks or diagonals adjacent to given square stretched in any direction
    // - Example vertical span (NORTH):
    //    ...
	//	|x|x|x|
	//	|x|x|x|
	//	| |s| |

    // Lookup tables
    // - We only consider NORTH / SOUTH directions and WEST / EAST for vertical and horizontal spans respectively
    // - All the other directions are being mapped into primary directions by getter functions
    // - NOTE: it is recommended to always use getters functions if possible
    extern Bitboard Vspans[SQUARE_RANGE][2];
    extern Bitboard Hspans[SQUARE_RANGE][2];

    template <Direction dir>
    inline Bitboard vertical_span(Square sq)
    {
        // Map nothern directions into 1, and everything else into 0
        return Vspans[sq][derived<NORTH>(dir)];
    }

    template <Direction dir>
    inline Bitboard horizontal_span(Square sq)
    {
        // Map eastern directions into 1, and everything else into 0
        return Hspans[sq][derived<EAST>(dir)];
    }

}