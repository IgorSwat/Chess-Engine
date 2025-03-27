#include "src/engine/engine.h"
#include "src/engine/pieces.h"
#include "src/engine/zobrist.h"
#include "src/engine/ttable.h"
#include "test/test.h"
#include <iostream>
#include <memory>

#ifdef USE_GUI
#include "src/gui/controller.h"
#endif


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
    // Testing::search_speed_test(12);

    // Quality test
    // Testing::search_accuracy_test(8, "test/data/search_test_data_custom.txt");

    // return 0;


    // ---------------
    // Main processing
    // ---------------

    std::unique_ptr<Board> board = std::make_unique<Board>();
    std::unique_ptr<Engine> engine = std::make_unique<Engine>(Engine::Mode::STANDARD);

    // An appropriate compilation argument decides whether to use command line or graphic interface
    #ifdef USE_GUI
        GUI::Textures::load_textures();

        std::unique_ptr<GUI::Controller> controller = std::make_unique<GUI::Controller>(board.get(), engine.get());
        
        controller->run();
    #else
        std::string fen;
        while (true) {

            std::cout << "Enter position FEN and depth:\n";
            std::cout << ">>> ";
            std::getline(std::cin, fen);
            if (fen.empty()) break;

            int depth;
            std::cout << ">>> ";
            std::cin >> depth;
            std::cin.get();

            engine->set_position(fen);
            auto [score, best_move] = engine->evaluate(Search::Depth(depth));
            std::cout << "\n---------- Search results (depth " << depth << ") ----------\n";
            std::cout << "- Best move: " << best_move << "\n" << std::dec;
            std::cout << "- Evaluation: " << score << "\n\n";
        }
    #endif

    return 0;
}