#include "src/engine/engine.h"
#include "src/engine/pieces.h"
#include "src/engine/zobrist.h"
#include "src/engine/ttable.h"
#include "test/test.h"
#include <iostream>
#include <memory>


int main()
{
    // --------------------
    // Initialization stage
    // --------------------

    Chessboard::initialize_board_space();
    Pieces::initialize_attack_tables();
    Zobrist::initialize_zobrist_numbers();
    // ....


    // -------------
    // Testing stage
    // -------------

    // bool test_results = Testing::run_tests();

    // if (!test_results)
    //     std::abort();

    // Speed test
    // Testing::search_speed_test(10);

    // Quality test
    // Testing::search_accuracy_test(10);


    // ---------------
    // Main processing
    // ---------------

    std::unique_ptr<Engine> engine = std::make_unique<Engine>(Engine::Mode::STANDARD);

    engine->set_position("r1b2rk1/1p5p/4pp2/p1P4q/1n2B3/4P1P1/5P1P/2QRNRK1 w - - 0 22");
    engine->evaluate(10);

    engine->show_ordering();

    // std::string fen;
    // while (true) {
    //     std::getline(std::cin, fen);
    //     if (fen.empty()) break;

    //     int depth;
    //     std::cin >> depth;
    //     std::cin.get();

    //     engine->set_position(fen);
    //     engine->evaluate(Search::Depth(depth));
    // }

    return 0;
}