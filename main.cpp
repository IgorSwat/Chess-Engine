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
	// Search::Depth depth = 8;
	// gui->addTester(std::make_unique<SearchPrinter>(engine.get(), true, depth));
	// gui->runTesters(true);

	// gui->run();		// Run GUI

	//tune(10, 10);

	// BoardConfig board;
	// board.loadPosition("r4rk1/pp3pp1/4p2p/1b1pP3/Nb1P4/1P2P3/1P3RPP/R3N1K1 w - - 0 18");
	// std::unique_ptr<Evaluation::Evaluator> evaluator = std::make_unique<Evaluation::Evaluator>(&board);
	// evaluator->evaluate();

	// std::vector<Move> moves = {
	// 	Move(SQ_A4, SQ_C5, QUIET_MOVE_FLAG),
	// 	Move(SQ_A4, SQ_C3, QUIET_MOVE_FLAG),
	// 	Move(SQ_E1, SQ_C2, QUIET_MOVE_FLAG),
	// 	Move(SQ_E1, SQ_D3, QUIET_MOVE_FLAG),
	// 	Move(SQ_A4, SQ_B6, QUIET_MOVE_FLAG),
	// 	Move(SQ_E1, SQ_F3, QUIET_MOVE_FLAG),
	// 	Move(SQ_A1, SQ_C1, QUIET_MOVE_FLAG),
	// 	Move(SQ_G2, SQ_G4, DOUBLE_PAWN_PUSH_FLAG)
	// };

	// for (Move move : moves) {
	// 	std::cout << move << std::dec << ", threats? " << evaluator->isCreatingThreats(move) << "\n";
	// }

	return 0;
}