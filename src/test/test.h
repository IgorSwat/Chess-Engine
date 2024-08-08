#pragma once

#include "../engine/engine.h"
#include <iostream>
#include <functional>
#include <cassert>
#include <tuple>
#include <initializer_list>


using TestingFunction = std::function<void(void)>;


// -----------------
// Testing framework
// -----------------

class Tester
{
public:
	Tester() {}

	void initEnvironment() const;

	void test(TestingFunction testingFunction) const { testingFunction(); }

	template <typename... TestingFunctions>
	void test(TestingFunction testingFunction, TestingFunctions... otherTests) const
	{
		if (environmentReady) {
			test(testingFunction);
			test(otherTests...);
		}
		else
			std::cerr << "[Tester]: Environment not initialized\n";
	}

private:
	static bool environmentReady;
};


namespace Testing {

	// ----------------
	// Type definitions
	// ----------------

	enum class OutputMode {
		CONSOLE = 1,
		FILE
	};


	// ------------
	// Test helpers
	// ------------

	// Search
	void print_search_tree(BoardConfig* board, const TranspositionTable* tTable, bool deep = false, OutputMode mode = OutputMode::CONSOLE);


	// --------------
	// Test functions
	// --------------

	// Magic bitboard tests
	void magicsTest();
	void xRayAttacksTest();

	// BoardConfig tests
	void staticLoadingTest();
	void pinsAndChecksTest();
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
	void zobristTest();

	// SEE tests
	void seeTest();

	// PGN parsing tests
	void pgnParsingTest();

	// Move selection tests
	void moveSelectionTest();

	// Search tests
	void searchTest1();
}