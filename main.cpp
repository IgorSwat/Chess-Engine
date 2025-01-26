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
	//tester.test(Testing::searchMultiTest);
	//tester.test(Testing::seeTest);
	//tester.test(Testing::moveSortingTest);
	tester.test(Testing::search_quality_test);

	// ----------------
	// Testing with GUI
	// ----------------

	// GUI::Textures::load_textures();		// Initialize textures
	// std::unique_ptr<GUI::BoardController> gui = std::make_unique<GUI::BoardController>();	// Create gui object

	// // Testers
	// std::unique_ptr<Engine> engine = std::make_unique<Engine>();	// Create engine
	// Search::Depth depth = 15;
	// gui->addTester(std::make_unique<SearchPrinter>(engine.get(), true, depth));
	// gui->runTesters(true);

	// gui->run();		// Run GUI

	//tune(10, 10);

	// BoardConfig board;
	// board.loadPosition("r5k1/1R3pp1/p7/n1rP2p1/P7/1B4PP/5PK1/7R w - - 3 34");

	// Evaluation::Evaluator evaluator(&board);
	// evaluator.evaluate();

	// MoveSelection::Selector selector(&board, &evaluator, MoveGeneration::QUIET);

	// selector.sort([&board, &evaluator](const Move& move) -> int32_t {
    //                        return 32 * evaluator.e_isAvoidingThreats_c(move) +
    //                        32 * evaluator.e_isCreatingThreats_c(move) +
    //                        128 * evaluator.e_isSafe_h(move);
    //             });

	// while (true) {
	// 	EnhancedMove move = selector.next(MoveSelection::Selector::STRICT);

	// 	if (move == Move::null())
	// 		break;

	// 	std::cout << move << std::dec << ", value: " << move.key() << "\n";
	// }

	return 0;
}