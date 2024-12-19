#include "test.h"
#include "../engine/engine.h"

namespace Testing {
    
    void show_all_moves(MoveSelection::Selector *selector)
    {
        Move move = selector->next();
        while (move != Move::null())
        {
            std::cout << move << "\n";
            move = selector->next();
        }
    }

    void moveSelectionTest()
    {
        const std::string fen = "r3k2r/p1q1ppbp/2p1bnp1/2pp2N1/4P3/N2P3P/PPP2PP1/R1BQR1K1 b kq - 2 10";

        BoardConfig board;
        board.loadPosition(fen);

        // Simple selection - start from quiet_checks
        MoveSelection::Selector selector(&board, nullptr, MoveGeneration::QUIET_CHECK, MoveSelection::SIMPLE_ORDERING);
        std::cout << "Simple selection:\n";
        show_all_moves(&selector);

        // Standard ordering
        selector = MoveSelection::Selector(&board, nullptr, MoveGeneration::CAPTURE, MoveSelection::STANDARD_ORDERING);
        std::cout << "\nStandard ordering:\n";
        show_all_moves(&selector);
    }

    void moveSortingTest()
    {
        BoardConfig board;
        Evaluation::Evaluator evaluator(&board);
        MoveSelection::Selector selector(&board, &evaluator, MoveGeneration::LEGAL);

        selector.sort([&evaluator, &board](const Move& move) -> int32_t {
            board.makeMove(move);
            Value eval = evaluator.evaluate();
            board.undoLastMove();
            return eval;
        });

        std::cout << "Sorting by static eval:\n";
        show_all_moves(&selector);
    }
}
