#pragma once

#include "misc.h"
#include <string>
#include <cassert>

constexpr Bitboard FILE_A = 0x0101010101010101;
constexpr Bitboard FILE_B = FILE_A << 1;
constexpr Bitboard FILE_C = FILE_A << 2;
constexpr Bitboard FILE_D = FILE_A << 3;
constexpr Bitboard FILE_E = FILE_A << 4;
constexpr Bitboard FILE_F = FILE_A << 5;
constexpr Bitboard FILE_G = FILE_A << 6;
constexpr Bitboard FILE_H = FILE_A << 7;
constexpr Bitboard FILES[8]{
	FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H
};

constexpr Bitboard ROW_1 = 0xFF;
constexpr Bitboard ROW_2 = ROW_1 << (8 * 1);
constexpr Bitboard ROW_3 = ROW_1 << (8 * 2);
constexpr Bitboard ROW_4 = ROW_1 << (8 * 3);
constexpr Bitboard ROW_5 = ROW_1 << (8 * 4);
constexpr Bitboard ROW_6 = ROW_1 << (8 * 5);
constexpr Bitboard ROW_7 = ROW_1 << (8 * 6);
constexpr Bitboard ROW_8 = ROW_1 << (8 * 7);
constexpr Bitboard ROWS[8]{
	ROW_1, ROW_2, ROW_3, ROW_4, ROW_5, ROW_6, ROW_7, ROW_8
};

constexpr Bitboard DIAG_A1H8 = 0x8040201008040201;
constexpr Bitboard DIAG_A8H1 = 0x0102040810204080;



namespace Bitboards {
	constexpr Bitboard POPCOUNT_K1 = 0x5555555555555555;
	constexpr Bitboard POPCOUNT_K2 = 0x3333333333333333;
	constexpr Bitboard POPCOUNT_K4 = 0x0f0f0f0f0f0f0f0f;
	constexpr Bitboard POPCOUNT_KF = 0x0101010101010101;
	constexpr Bitboard MIRROR_HORIZONTAL_K1 = 0x5555555555555555;
	constexpr Bitboard MIRROR_HORIZONTAL_K2 = 0x3333333333333333;
	constexpr Bitboard MIRROR_HORIZONTAL_K4 = 0x0f0f0f0f0f0f0f0f;

	std::string bitboardToString(Bitboard bb);

	constexpr inline int popcount(Bitboard x)
	{
		x = x - ((x >> 1) & POPCOUNT_K1);
		x = (x & POPCOUNT_K2) + ((x >> 2) & POPCOUNT_K2);
		x = (x + (x >> 4)) & POPCOUNT_K4;
		x = (x * POPCOUNT_KF) >> 56;
		return static_cast<int>(x);
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

	inline Bitboard mirrorVertical(Bitboard x)
	{
		return _byteswap_uint64(x);
	}

	constexpr inline Bitboard mirrorHorizontal(Bitboard x)
	{
		x = ((x >> 1) & MIRROR_HORIZONTAL_K1) | ((x & MIRROR_HORIZONTAL_K1) << 1);
		x = ((x >> 2) & MIRROR_HORIZONTAL_K2) | ((x & MIRROR_HORIZONTAL_K2) << 2);
		x = ((x >> 4) & MIRROR_HORIZONTAL_K4) | ((x & MIRROR_HORIZONTAL_K4) << 4);
		return x;
	}
}