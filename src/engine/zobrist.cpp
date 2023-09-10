#include "zobrist.h"
#include "../logic/boardconfig.h"
#include <unordered_set>
#include <cstdlib>
#include <random>
#include <algorithm>

namespace {
	constexpr unsigned int randSeed = 410375;

	constexpr int squareFactor = 8;
	constexpr int piecePlacementFactor = 64;
	constexpr int sidePiecePlacementFactor = 384;
	constexpr int sideToMoveID = 768;
	constexpr int castlingRightsID = 769;
	constexpr int castlingSideFactor = 2;
	constexpr int enPassantID = 773;
}

U64 ZobristHash::hashCodes[]{};
bool ZobristHash::initFlag = false;



ZobristHash::ZobristHash()
{
	initHashCodes();
}

void ZobristHash::generateHash(BoardConfig* config)
{
	hash = 0LL;
	for (int i = 0; i < 32; i++)
	{
		const Piece* piece = config->getPiece(i);
		if (piece->isActive())
			updateHash(piece->getColor(), piece->getType(), piece->getPosition());
	}
	if (config->getSideOnMove() == BLACK)
		updateHashBySideToMoveChange();
	if (config->hasCastlingRight(WHITE, SHORT_CASTLE))
		updateHash(WHITE, SHORT_CASTLE);
	if (config->hasCastlingRight(WHITE, LONG_CASTLE))
		updateHash(WHITE, LONG_CASTLE);
	if (config->hasCastlingRight(BLACK, SHORT_CASTLE))
		updateHash(BLACK, SHORT_CASTLE);
	if (config->hasCastlingRight(BLACK, LONG_CASTLE))
		updateHash(BLACK, LONG_CASTLE);
	if (config->getEnPassantLine() >= 0)
		updateHash(config->getEnPassantLine());
}

void ZobristHash::updateHash(Side pieceColor, PieceType pieceType, const Square& square)
{
	hash ^= hashCodes[pieceColor * sidePiecePlacementFactor + pieceType * piecePlacementFactor + square.y * squareFactor + square.x];
}

void ZobristHash::updateHash(Side side, CastleType castleType)
{
	hash ^= hashCodes[castlingRightsID + side * castlingSideFactor + castleType];
}

void ZobristHash::updateHash(int enPassantRow)
{
	hash ^= hashCodes[enPassantID + enPassantRow];
}

void ZobristHash::updateHashBySideToMoveChange()
{
	hash ^= hashCodes[sideToMoveID];
}

void ZobristHash::initHashCodes()
{
	if (!initFlag)
	{
		std::mt19937_64 gen(randSeed);
		std::uniform_int_distribution<U64> distribution(0, INT64_MAX);
		std::unordered_set<U64> generatedCodes;
		std::generate(hashCodes, hashCodes + maxCodesNum, [&generatedCodes, &gen, &distribution]() {
			U64 code = 0LL;
			do
				code = distribution(gen);
			while (generatedCodes.find(code) != generatedCodes.end());
			generatedCodes.insert(code);
			return code;
		});
		initFlag = true;
	}
}