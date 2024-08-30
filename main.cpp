#include "src/test/test.h"
#include "src/test/testEngine.h"
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
	//tester.test(Testing::seeTest);

	// ----------------
	// Testing with GUI
	// ----------------

	GUI::Textures::load_textures();		// Initialize textures
	std::unique_ptr<GUI::BoardController> gui = std::make_unique<GUI::BoardController>();	// Create gui object

	// Testers
	std::unique_ptr<TestEngine> engine = std::make_unique<TestEngine>();	// Create engine
	Search::Depth depth = 7;
	gui->addTester(std::make_unique<SearchPrinter>(engine.get(), true, depth));
	gui->runTesters(true);

	gui->run();		// Run GUI

	// BoardConfig board;
	// board.loadPosition("rnb1kbnr/ppp2ppp/3p4/8/3NP2q/8/PPP2PPP/RNBQKB1R w KQkq - 1 5");

	// Evaluation::Evaluator evaluator(&board);

	// std::cout << "Static eval: " << Search::relative_score(evaluator.evaluate(), &board) << "\n";

	// MoveSelector selector(&board);
	// selector.generateMoves<MoveGeneration::LEGAL>();

	// Move move = selector.selectNext<GenerationStrategy::STRICT, SelectionStrategy::SIMPLE>();
	// while (move != Move::null()) {
	// 	move = selector.selectNext<GenerationStrategy::STRICT, SelectionStrategy::SIMPLE>();
	// }


	return 0;
}