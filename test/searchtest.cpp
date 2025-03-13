#include "test.h"
#include "../src/engine/engine.h"
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
        Board board;
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
            board.load_position(positions[i]);
            engine->set_position(board);

            std::cout << "----- Position " << i + 1 << " -----\n";
            engine->evaluate(depth);
            std::cout << "\n";
        }
    }

}