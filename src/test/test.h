#pragma once
#include "../logic/boardConfig.h"
#include "../logic/pieces.h"
#include "../logic/zobrist.h"
#include "../engine/moveGeneration.h"
#include <functional>


using TestingFunction = std::function<void(void)>;

struct MoveCounters
{
	uint64_t noMoves = 0;
	uint64_t noCaptures = 0;
	uint64_t noEnpassants = 0;
	uint64_t noCastles = 0;
	uint64_t noPromotions = 0;

	void operator+=(const MoveCounters& other)
	{
		noMoves += other.noMoves;
		noCaptures += other.noCaptures;
		noEnpassants += other.noEnpassants;
		noCastles += other.noCastles;
		noPromotions += other.noPromotions;
	}
};

struct PerftData
{
	std::string fen;
	int depth;
	MoveCounters moveCounters;
};



class Tester
{
public:
	Tester()
	{
		Pieces::initAttackTables();
		Board::initBoardElements();
		Zobrist::initZobristHashing();
	}

	void test(TestingFunction testingFunction)
	{
		testingFunction();
	}

	template <typename... TestingFunctions>
	void test(TestingFunction testingFunction, TestingFunctions... otherTests)
	{
		test(testingFunction);
		test(otherTests...);
	}
};



namespace Testing {
	// Global chess logic tests
	void testMagics();
	void testXRayAttacks();

	// BoardConfig tests
	void staticLoadingTest();
	void pinsAndChecksBoardconfigTestRuySteinitz();
	void castlingRightsAndEnPassantTest();

	// Move generation tests
	void perftMovegenTestStartingPos();
	void perftMovegenTestMidgamePos1();
	void perftMovegenTestMidgamePos2();
	void perftMovegenTestMidgamePos3();
	void perftMovegenTestEndgamePos();

	// Performance tests
	template <bool deep>
	void perftSpeedTestStartingPos();
	template <bool deep>
	void perftSpeedTestMidgamePos();
	template <bool deep>
	void perftSpeedTestEndgamePos();

	// Universal PGN tests
	void testZobrist();
}