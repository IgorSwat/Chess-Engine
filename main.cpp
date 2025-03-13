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

    // ---------------
    // Main processing
    // ---------------

    Testing::search_speed_test(7);

    // std::unique_ptr<Engine> engine = std::make_unique<Engine>(Engine::Mode::TRACE);

    // std::string fen;
    // while (true) {
    //     std::getline(std::cin, fen);
    //     if (fen.empty()) break;

    //     engine->set_position(fen);
    //     engine->evaluate(10);
    // }

    return 0;
}