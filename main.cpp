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

	// Testers
	//std::unique_ptr<Engine> engine = std::make_unique<Engine>();	// Create engine
	//Search::Depth depth = 7;
	//gui->addTester(std::make_unique<SearchPrinter>(engine.get(), true, depth));
	//gui->runTesters(true);

	//gui->run();		// Run GUI

	//tune(10, 10);

	// BoardConfig board;
	// board.loadPosition("r1bq2k1/ppppbrpp/8/3NPp1Q/4pB2/8/PPP2PPP/R3R1K1 b - - 3 15");

	// std::cout << board.fullLegalityTest(Move(SQ_G7, SQ_G5, DOUBLE_PAWN_PUSH_FLAG)) << "\n";

	// std::unique_ptr<Evaluation::Evaluator> evaluator = std::make_unique<Evaluation::Evaluator>(&board);
	// evaluator->evaluate();

	// MoveSelection::Selector selector(&board, evaluator.get(), MoveGeneration::CAPTURE, true);
	// MoveSelection::improved_ordering(selector);

	// Move move = selector.next();
	// while (move != Move::null()) {
	// 	if (!board.fullLegalityTest(move))
	// 		std::cout << "XD\n";
	// 	move = selector.next();
	// }

	return 0;
}