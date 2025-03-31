#pragma once

#include "types.h"
#include <string>
#include <nmmintrin.h>


/*
    ---------- Bitboards ----------

    A general purpose bitboard operation library.
    - Bitboard is a 64-bit unsigned integer that represents a given aspect of board state or geometry (like placement of pieces).
    - Uses MSVC-specyfic functions to speed up calculations
*/

namespace Bitboards {

    // ------------------------------------------
	// Local bitboard operations - bit extraction
	// ------------------------------------------

    inline Square lsb(Bitboard mask)
	{
	#ifdef _MSC_VER
		unsigned long bitID;
		_BitScanForward64(&bitID, mask);
	#else
		unsigned long bitID = __builtin_ctzll(mask);
	#endif
		return Square(bitID);
	}

	inline Square msb(Bitboard mask)
	{
	#ifdef _MSC_VER
		unsigned long bitID;
		_BitScanReverse64(&bitID, mask);
	#else
		unsigned long bitID = 63 - __builtin_clzll(mask);
	#endif
		return Square(bitID);
	}

	inline Square pop_lsb(Bitboard& mask)
	{
		Square sq = lsb(mask);
		mask &= (mask - 1);

		return sq;
	}


	// --------------------------------------
	// Global bitboard operations - bit count
	// --------------------------------------

	inline unsigned popcount(Bitboard bb)
	{
	#ifdef _MSC_VER
		return uint32_t(__popcnt64(bb));
	#else
		return __builtin_popcountll(bb);
	#endif
	}

	inline bool singly_populated(Bitboard bb)
	{
		return bb && !(bb & (bb - 1));
	}


	// -----------------------------------
	// Global bitboard operations - shifts
	// -----------------------------------

	// Static (compile time) shift
	template <Direction dir>
	constexpr inline Bitboard shift_s(Bitboard bb)
	{
		// We need to use "magic numbers" here, because constants from boardspace.h cannot be included to avoid header include loop
		return dir == NORTH ? bb << 8 : dir == SOUTH ? bb >> 8 :
			   dir == EAST ? (bb & 0x7f7f7f7f7f7f7f7f) << 1 : dir == WEST ? (bb & 0xfefefefefefefefe) >> 1 :
			   dir == NORTH_EAST ? (bb & 0x7f7f7f7f7f7f7f7f) << 9 : dir == NORTH_WEST ? (bb & 0xfefefefefefefefe) << 7 :
			   dir == SOUTH_EAST ? (bb & 0x7f7f7f7f7f7f7f7f) >> 7 : dir == SOUTH_WEST ? (bb & 0xfefefefefefefefe) >> 9 : 0;
	}

	// Dynamic (runtime) shift
	constexpr inline Bitboard shift_d(Bitboard bb, Direction dir)
	{
		// We need to use "magic numbers" here, because constants from boardspace.h cannot be included to avoid header include loop
		bb = derived<EAST>(dir) ? (bb & 0x7f7f7f7f7f7f7f7f) :
			 derived<WEST>(dir) ? (bb & 0xfefefefefefefefe) : bb;
		
		return dir > 0 ? bb << dir : bb >> -dir;
	}


	// ----------------------------------
	// Global bitboard operations - fills
	// ----------------------------------

	// General fill algorithm
	template <Direction dir>
	constexpr inline Bitboard fill(Bitboard bb)
	{
		Bitboard shifted = bb;

		// In general, we can describe fill operation as many shifts performed in the same direction
		do {
			bb = shifted;
			shifted |= shift_s<dir>(shifted);
		} while (shifted != bb);

		return shifted;
	}

	// Specyfic, efficient implementation for NORTH direction
	// Uses divide and conquer method
	template <>
	constexpr inline Bitboard fill<NORTH>(Bitboard bb)
	{
		bb |= (bb << 32);
		bb |= (bb << 16);
		bb |= (bb << 8);

		return bb;
	}

	// Specyfic, efficient implementation for SOUTH direction
	// Uses divide and conquer method
	template <>
	constexpr inline Bitboard fill<SOUTH>(Bitboard bb)
	{
		bb |= (bb >> 32);
		bb |= (bb >> 16);
		bb |= (bb >> 8);
		
		return bb;
	}


	// ------------------------------------------
	// Global bitboard operations - miscellaneous
	// ------------------------------------------

	// NOTE: it's not optimized too much, should be used only for debugging purposes
	std::string to_string(Bitboard bb);

}