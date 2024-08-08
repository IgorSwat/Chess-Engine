#include "src/test/test.h"
#include "src/gui/boardController.h"
#include "src/logic/zobrist.h"
#include "src/engine/moveSelection.h"
#include <iostream>
#include <iomanip>
#include <memory>
#include <chrono>

using namespace Testing;
	

int main()
{
	Tester tester;
	tester.initEnvironment();

	// ---------------------------
	// Testing with test functions
	// ---------------------------
	
	//tester.test(Testing::perftMovegenTestEndgamePos, Testing::perftMovegenTestMidgamePos2, Testing::pinsAndChecksTest);
	//tester.test(Testing::searchTest1);


	// ----------------
	// Testing with GUI
	// ----------------

	GUI::Textures::load_textures();		// Initialize textures
	std::unique_ptr<GUI::BoardController> gui = std::make_unique<GUI::BoardController>();	// Create gui object

	// Testers
	std::unique_ptr<Engine> engine = std::make_unique<Engine>();	// Create engine
	Search::Depth depth = 3;
	gui->addTester(std::make_unique<SearchPrinter>(engine.get(), depth));
	gui->runTesters(true);

	gui->run();		// Run GUI


	return 0;
}