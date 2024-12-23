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
	//tester.test(Testing::moveSortingTest);

	// ----------------
	// Testing with GUI
	// ----------------

	// GUI::Textures::load_textures();		// Initialize textures
	// std::unique_ptr<GUI::BoardController> gui = std::make_unique<GUI::BoardController>();	// Create gui object

	// // Testers
	// std::unique_ptr<Engine> engine = std::make_unique<Engine>();	// Create engine
	// Search::Depth depth = 7;
	// gui->addTester(std::make_unique<SearchPrinter>(engine.get(), true, depth));
	// gui->runTesters(true);

	// gui->run();		// Run GUI

	//tune(10, 10);

	// BoardConfig board;
	// board.loadPosition("r1b4r/1pp1k1p1/p2p3p/1NbB2Nq/3nP1nP/3Q4/PP3PP1/R1B1K2R w KQ - 0 14");
	// std::unique_ptr<Evaluation::Evaluator> evaluator = std::make_unique<Evaluation::Evaluator>(&board);
	// evaluator->evaluate();

	// MoveSelection::Selector selector(&board, evaluator.get(), MoveGeneration::CAPTURE, MoveSelection::IMPROVED_ORDERING, true);

	// selector.hasNext();

	// Move move = selector.next();
	// while (move != Move::null()) {
	// 	std::cout << move << "\n";
	// 	move = selector.next();
	// }

	return 0;
}