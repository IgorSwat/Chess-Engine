#include "test.h"
#include "../src/engine/engine.h"
#include "../src/utilities/parsing.h"
#include <chrono>
#include <fstream>
#include <memory>
#include <vector>

// Since we cannot really perform unit tests on search heuristics, these tests have different format
// - We are focusing on search speed and accuracy in finding appropriate moves
// - NOTE: to run those tests, simply invoke appropriate function inside the main function call

namespace Testing {

    // This test focuses on measuring engine's speed
    // - We do not check the return value in any way
    void search_speed_test(Search::Depth depth)
    {
        std::unique_ptr<Engine> engine = std::make_unique<Engine>(Engine::Mode::STATS);

        // A few different positions
        std::vector<std::string> positions = {
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",                 // Starting position
            "r1bq2k1/ppppbrpp/8/4Pp1Q/4pB2/2N5/PPP2PPP/R3R1K1 w - - 2 15",              // Midgame position, calm
            "1r1q1rk1/3bppbp/3p2pB/1np2P2/p2nP1P1/2NP1N1P/PP1Q2B1/1R3RK1 w - - 3 19",   // Midgame position, complex
            "rn1q1rk1/ppp1bpp1/5n1p/4p1P1/4p1bP/1PN5/PBPPQP2/2KR1BNR w - - 1 10",       // Midgame position, complex
            "R7/8/4k3/3n4/1p4P1/5P2/5K2/8 w - - 0 51",                                  // Endgame position, simple
            "8/4kbp1/5p2/5Q1p/8/8/5K2/8 w - - 1 51",                                    // Endgame position, complex
        };

        for (int i = 0; i < positions.size(); i++) {
            engine->set_position(positions[i]);

            std::cout << "----- Position " << i + 1 << " -----\n";
            engine->evaluate(depth);
            std::cout << "\n";
        }
    }

    // This test measures both search speed and accuracy
    // - Accuracy is measured by comparing engine's first choice suggestions to best moves pointed out in data file
    void search_accuracy_test(Search::Depth depth, std::string input)
    {
        std::unique_ptr<Engine> engine = std::make_unique<Engine>(Engine::Mode::STANDARD);

        // Load input file
        std::ifstream input_file(input);

        // Score & time variables
        unsigned no_tests = 0;
        unsigned total_score = 0, max_score = 0;
        std::chrono::duration<double> search_time = std::chrono::milliseconds(0);

        // Input file contains of the following entries:
        // - Number of acceptable moves, position (fen), list of acceptable moves with corresponding scores
        // - Each score is from 1 to 10
        // - A move not mentioned on acceptable move list gets 0 points
        while (!input_file.eof()) {
            no_tests++;
            max_score += 10;

            // Number of acceptable moves (those which get any points)
            unsigned no_moves;
            input_file >> no_moves;

            // Parse FEN notation of the position
            // - NOTE: additional get() is required to get rid of '\n' symbol from move number line
            std::string fen;
            input_file.get();
            std::getline(input_file, fen);

            // Load position
            engine->set_position(fen);

            // Evaluate and measure time
            auto start = std::chrono::steady_clock::now();
            auto [evaluation, best_move] = engine->evaluate(depth);
            auto end = std::chrono::steady_clock::now();

            search_time += end - start;

            // Now check all the acceptable moves and decide how many points is engine's response worth
            std::string move_notation;
            int move_points;
            bool accepted = false;

            for (unsigned i = 0; i < no_moves; i++) {
                input_file >> move_notation;
                input_file >> move_points;

                // Parse move from notation
                Move move = Parsing::parse_move(*engine->mem_board(), move_notation);

                if (i == 0)
                    std::cout << std::dec << "Test " << no_tests << "  |  best: " << move << "  |  found: " <<  best_move << "\n";
                
                // Update score
                if (!accepted && move == best_move) {
                    total_score += move_points;
                    accepted = true;
                }
            }
        }

        std::cout << std::dec << "---------------------------------------------------\n";
        std::cout << "Test cases: " << no_tests << "\n";
        std::cout << "Total score: " << total_score << "/" << max_score << " (" << total_score * 100 / max_score << "%)\n";
        std::cout << "Total search time: " << search_time.count() << " seconds \n";
    }

}