#include "test.h"
#include "../engine/moveSelection.h"

namespace Testing {
    
    template <GenerationStrategy genStrategy, SelectionStrategy selStrategy>
    void show_all_moves(MoveSelector *selector)
    {
        Move move = selector->selectNext<genStrategy, selStrategy>();
        while (move != Move::null())
        {
            std::cout << move << "\n";
            move = selector->selectNext<genStrategy, selStrategy>();
        }
    }

    void moveSelectionTest()
    {
        const std::string fen = "r3k2r/p1q1ppbp/2p1bnp1/2pp2N1/4P3/N2P3P/PPP2PP1/R1BQR1K1 b kq - 2 10";

        BoardConfig board;
        MoveSelector selector(&board);
        board.loadPosition(fen);

        // Simple selection - start from quiet_checks
        selector.generateMoves<MoveGeneration::QUIET_CHECK>();
        std::cout << "Simple selection:\n";
        show_all_moves<GenerationStrategy::CASCADE, SelectionStrategy::SIMPLE>(&selector);

        // Standard ordering
        selector.generateMoves<MoveGeneration::CAPTURE>();
        std::cout << "\nStandard ordering:\n";
        show_all_moves<GenerationStrategy::CASCADE, SelectionStrategy::STANDARD_ORDERING>(&selector);
    }
}
