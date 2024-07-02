#pragma once

#include "boardElements.h"
#include <string>
#include <cassert>
#include <nmmintrin.h>


namespace Bitboards {

	// --------------------------
	// Bitboard population issues
	// --------------------------

	inline bool single_populated(Bitboard bb)
	{
		return !(bb & (bb - 1));
	}

	inline int popcount(Bitboard x)
	{
		return int(__popcnt64(x));
	}


	// -------------------
	// LSB and MSB getters
	// -------------------

	inline Square lsb(Bitboard mask)
	{
		assert(mask);	// TODO: REMOVE in future
		unsigned long bitID;
		_BitScanForward64(&bitID, mask);
		return Square(bitID);
	}

	inline Square msb(Bitboard mask)
	{
		assert(mask);	// TODO: REMOVE in future
		unsigned long bitID;
		_BitScanReverse64(&bitID, mask);
		return Square(bitID);
	}

	inline Square pop_lsb(Bitboard& mask)
	{
		Square sq = lsb(mask);
		mask &= (mask - 1);
		return sq;
	}


	// --------------------------------
	// Bitboard direction-wise shifting
	// --------------------------------

	// Static, compile time shift
	template <Direction dir>
	constexpr inline Bitboard shift_s(Bitboard bb)
	{
		return dir == NORTH ? bb << 8 : dir == SOUTH ? bb >> 8 :
			dir == EAST ? (bb & Board::NOT_FILE_H) << 1 : dir == WEST ? (bb & Board::NOT_FILE_A) >> 1 :
			dir == NORTH_EAST ? (bb & Board::NOT_FILE_H) << 9 : dir == NORTH_WEST ? (bb & Board::NOT_FILE_A) << 7 :
			dir == SOUTH_EAST ? (bb & Board::NOT_FILE_H) >> 7 : dir == SOUTH_WEST ? (bb & Board::NOT_FILE_A) >> 9 : 0;
	}

	// Dynamic, runtime shift
	Bitboard shift_d(Bitboard bb, Direction dir);


	// -------------
	// Bitboard fill
	// -------------

	// Generic implementation for any direction
	template <Direction dir> constexpr inline Bitboard fill(Bitboard bb)
	{
		Bitboard shifted = shift_s<dir>(bb);
		while ((bb ^ shifted) & (~bb)) {
			bb |= shifted;
			shifted = shift_s<dir>(bb);
		}
		return bb;
	}

	// Specyfic, efficient implementations for vertical directions
	template <> constexpr inline Bitboard fill<NORTH>(Bitboard bb)
	{
		bb |= (bb << 32);
		bb |= (bb << 16);
		bb |= (bb << 8);
		return bb;
	}

	template <> constexpr inline Bitboard fill<SOUTH>(Bitboard bb)
	{
		bb |= (bb >> 32);
		bb |= (bb >> 16);
		bb |= (bb >> 8);
		return bb;
	}

	// Collapses all files bits into the one row to obtain small index
	constexpr inline int clp_files_index(Bitboard bb)
	{
		return int(fill<SOUTH>(bb) & 0xff);
	}


	// -------------
	// Bitboard misc
	// -------------

	std::string to_string(Bitboard bb);

}