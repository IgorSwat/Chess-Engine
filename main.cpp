#include "src/test/test.h"
#include "src/gui/boardController.h"
#include "src/logic/zobrist.h"
#include "src/engine/moveSelection.h"
#include "src/utilities/tuning.h"
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
	tester.test(Testing::searchMultiTest);
	//tester.test(Testing::seeTest);

	// ----------------
	// Testing with GUI
	// ----------------

	// GUI::Textures::load_textures();		// Initialize textures
	// std::unique_ptr<GUI::BoardController> gui = std::make_unique<GUI::BoardController>();	// Create gui object

	// // Testers
	// std::unique_ptr<Engine> engine = std::make_unique<Engine>();	// Create engine
	// Search::Depth depth = 8;
	// gui->addTester(std::make_unique<SearchPrinter>(engine.get(), true, depth));
	// gui->runTesters(true);

	// gui->run();		// Run GUI

	//tune(10, 10);

	// BoardConfig board;
	// board.loadPosition("2k4r/Q3Rp2/1pq4p/2p3p1/8/6P1/PKP2rP1/3R4 b - - 1 24");
	// std::unique_ptr<Evaluation::Evaluator> evaluator = std::make_unique<Evaluation::Evaluator>(&board);

	// std::cout << std::dec << "Eval: " << evaluator->evaluate() << "\n";

	return 0;
}