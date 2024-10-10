#include "test.h"
#include "../engine/engine.h"

namespace Testing {
    
    void show_all_moves(MoveSelector *selector)
    {
        Move move = selector->selectNext(true);
        while (move != Move::null())
        {
            std::cout << move << "\n";
            move = selector->selectNext(true);
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
        selector.setStrategy(MoveSelection::SIMPLE_ORDERING);
        std::cout << "Simple selection:\n";
        show_all_moves(&selector);

        // Standard ordering
        selector.generateMoves<MoveGeneration::CAPTURE>();
        selector.setStrategy(MoveSelection::STANDARD_ORDERING);
        std::cout << "\nStandard ordering:\n";
        show_all_moves(&selector);
    }

    void moveSortingTest()
    {
        BoardConfig board;
        MoveSelector selector(&board);
        Evaluation::Evaluator evaluator(&board);

        selector.generateMoves<MoveGeneration::LEGAL>();
        MoveSelection::sort_moves(selector, [&evaluator, &board](const Move& move) -> int {
            board.makeMove(move);
            Value eval = evaluator.evaluate();
            board.undoLastMove();
            return eval;
        });

        std::cout << "Sorting by static eval:\n";
        show_all_moves(&selector);
    }
}
