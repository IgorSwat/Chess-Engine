#pragma once

#include "boardElements.h"
#include <string>
#include <cassert>


// Some general bitwise operations	
namespace Bitboards {
	constexpr Bitboard POPCOUNT_K1 = 0x5555555555555555;
	constexpr Bitboard POPCOUNT_K2 = 0x3333333333333333;
	constexpr Bitboard POPCOUNT_K4 = 0x0f0f0f0f0f0f0f0f;
	constexpr Bitboard POPCOUNT_KF = 0x0101010101010101;
	constexpr Bitboard MIRROR_HORIZONTAL_K1 = 0x5555555555555555;
	constexpr Bitboard MIRROR_HORIZONTAL_K2 = 0x3333333333333333;
	constexpr Bitboard MIRROR_HORIZONTAL_K4 = 0x0f0f0f0f0f0f0f0f;

	std::string bitboardToString(Bitboard bb);

	inline bool isSinglePopulated(Bitboard bb)
	{
		return !(bb & (bb - 1));
	}

	inline int popcount(Bitboard x)
	{
		return int(__popcnt64(x));
	}

	inline Square lsb(Bitboard mask)
	{
		assert(mask);	// REMOVE in future
		unsigned long bitID;
		_BitScanForward64(&bitID, mask);
		return Square(bitID);
	}

	inline Square msb(Bitboard mask)
	{
		assert(mask);	// REMOVE in future
		unsigned long bitID;
		_BitScanReverse64(&bitID, mask);
		return Square(bitID);
	}

	inline Square popLsb(Bitboard& mask)
	{
		Square sq = lsb(mask);
		mask &= (mask - 1);
		return sq;
	}

	inline Bitboard mirrorVertical(Bitboard x)
	{
		return _byteswap_uint64(x);
	}

	inline Bitboard mirrorHorizontal(Bitboard x)
	{
		x = ((x >> 1) & MIRROR_HORIZONTAL_K1) | ((x & MIRROR_HORIZONTAL_K1) << 1);
		x = ((x >> 2) & MIRROR_HORIZONTAL_K2) | ((x & MIRROR_HORIZONTAL_K2) << 2);
		x = ((x >> 4) & MIRROR_HORIZONTAL_K4) | ((x & MIRROR_HORIZONTAL_K4) << 4);
		return x;
	}

	inline Bitboard shift(Bitboard bb, Direction dir)
	{
		return dir == NORTH ? bb << 8 : dir == SOUTH ? bb >> 8 :
			dir == EAST ? (bb & NOT_FILE_H) << 1 : dir == WEST ? (bb & NOT_FILE_A) >> 1 :
			dir == NORTH_EAST ? (bb & NOT_FILE_H) << 9 : dir == NORTH_WEST ? (bb & NOT_FILE_A) << 7 :
			dir == SOUTH_EAST ? (bb & NOT_FILE_H) >> 7 : dir == SOUTH_WEST ? (bb & NOT_FILE_A) >> 9 : 0;
	}
}