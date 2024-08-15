#include "zobrist.h"
#include "boardConfig.h"
#include "randomGens.h"
#include <unordered_set>
#include <initializer_list>
#include <numeric>
#include <algorithm>


namespace Zobrist {

    U64 hashCodes[HASH_CODES_NUM] = { 0 };

    // ----------------------------
    // Hamming distance calculation
    // ----------------------------

    int hammingDistance(U64 code1, U64 code2)
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

    float averageHammingDistance(const U64 codes[], int n)
    {
        int numOfPairs = n * (n - 1) / 2;
		int hammingDistanceSum = 0;
		for (int i = 0; i < n; i++)
		{
			for (int j = i + 1; j < n; j++)
				hammingDistanceSum += hammingDistance(codes[i], codes[j]);
		}
		return (float)hammingDistanceSum / numOfPairs;
    }

    int minimalHammingDistance(const U64 codes[], int n)
    {
        int minHammingDistance = std::numeric_limits<int>::max();
		for (int i = 0; i < n; i++)
		{
			for (int j = i + 1; j < n; j++)
				minHammingDistance = std::min(minHammingDistance, hammingDistance(codes[i], codes[j]));
		}
		return minHammingDistance;
    }


    // --------------------
    // Zobrist initializers
    // --------------------

    // Global initialization for prn codes
    void initZobristHashing()
    {
        constexpr unsigned int SEED = 410376;

        MersenneTwister<uint64_t> generator(SEED);
        std::unordered_set<U64> generatedCodes;
        
        std::generate(hashCodes, hashCodes + HASH_CODES_NUM, [&generatedCodes, &generator]() {
            U64 code = 0ULL;
            do
                code = generator.random();
            while (generatedCodes.find(code) != generatedCodes.end());
            generatedCodes.insert(code);
            return code;
        });
    }


    // -----------------------------
    // Zobrist container and methods
    // -----------------------------

    void ZobristHash::generateHash(BoardConfig* board)
    {
        hash = 0ULL;

        // Piece placement hashing
        for (int sq = SQ_A1; sq <= SQ_H8; ++sq) {
            Piece piece = board->onSquare(Square(sq));
            if (piece != NO_PIECE)
                updateByPlacementChange(piece, Square(sq));
        }

        // Other parameters hashing
        if (board->movingSide() == BLACK)
            updateBySideOnMoveChange();
        updateByCastlingRightsChange(board->castlingRights());
        updateByEnpassantChange(board->enpassantSquare());
    }

}