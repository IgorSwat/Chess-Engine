#pragma once

#include "../logic/piece.h"
#include <cinttypes>
#include <iostream>

class BoardConfig;
using U64 = std::uint64_t;

namespace Zobrist {
	template <typename IntType>
	int calculateHammingDistance(IntType code1, IntType code2)
	{
		int hammingDistance = 0;
		while (code1 > 0 || code2 > 0)
		{
			if (code1 % 2 != code2 % 2)
				hammingDistance += 1;
			code1 /= 2;
			code2 /= 2;
		}
		return hammingDistance;
	}

	template <typename Container>
	float calculateAverageHammingDistance(Container& codes, int n, int maxHammingDistance)
	{
		int numOfPairs = n * (n - 1) / 2;
		int hammingDistanceSum = 0;
		for (int i = 0; i < n; i++)
		{
			for (int j = i + 1; j < n; j++)
				hammingDistanceSum += calculateHammingDistance(codes[i], codes[j]);
		}
		return (float)hammingDistanceSum / numOfPairs;
	}

	template <typename Container>
	float calculateMinimalHammingDistance(Container& codes, int n, int maxHammingDistance)
	{
		int minHammingDistance = maxHammingDistance;
		for (int i = 0; i < n; i++)
		{
			for (int j = i + 1; j < n; j++)
				minHammingDistance = std::min(minHammingDistance, calculateHammingDistance(codes[i], codes[j]));
		}
		return minHammingDistance;
	}
}



class ZobristHash
{
public:
	ZobristHash();
	~ZobristHash() = default;

	void generateHash(BoardConfig* config);
	void updateHash(Side pieceColor, PieceType pieceType, const Square& square);
	void updateHash(Side side, CastleType castleType);
	void updateHash(int enPassantRow);
	void updateHashBySideToMoveChange();
	U64 getHash() const { return hash; }
private:
	static void initHashCodes();
	static constexpr int maxCodesNum = 781;
	static U64 hashCodes[maxCodesNum];
	static bool initFlag;

	U64 hash = 0LL;
};