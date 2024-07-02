#include "src/test/test.h"
//#include "src/gui/boardController.h"
#include "src/logic/zobrist.h"
//#include "src/engine/engine.h"
#include <iostream>
#include <iomanip>
#include <memory>
#include <chrono>

using namespace Testing;
	

int main()
{
	Tester tester;
	tester.initEnvironment();
	
	tester.test(Testing::pgnParsingTest);
	//tester.test(Testing::perftMovegenTestMidgamePos1);

	//runGUI();

	return 0;
}