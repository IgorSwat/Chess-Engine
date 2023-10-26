#pragma once
#include "../logic/boardConfig.h"
#include "../logic/pieces.h"
#include <functional>

namespace Testing {
	// Global chess logic tests
	void testMagics();
	void testXRayAttacks();

	// BoardConfig tests
	void pinsAndChecksBoardconfigTestRuySteinitz();
}

using TestingFunction = std::function<void(void)>;

class Tester
{
public:
	Tester()
	{
		Pieces::initAttackTables();
		initBoardElements();
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