#include "test.h"
#include "../src/engine/board.h"
#include "../src/engine/evalconfig.h"


namespace Testing {

    // -------------------------------
    // Static Exchange Evaluation test
    // -------------------------------

    REGISTER_TEST(see_test)
    {
        Board::Board board;

        // Position 1
        board.load_position("1k1r4/1pp4p/p7/4p3/8/P5P1/1PP4P/2K1R3 w - - 0 1");
        Evaluation::Eval gain1 = Evaluation::PieceValues[PAWN];
        ASSERT_EQUALS(gain1, board.see(Move(SQ_E1, SQ_E5, Moves::CAPTURE_FLAG)));

        // Position 2
        board.load_position("1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - - 0 1");
        Evaluation::Eval gain2 = Evaluation::PieceValues[PAWN] - Evaluation::PieceValues[KNIGHT];
        ASSERT_EQUALS(gain2, board.see(Move(SQ_D3, SQ_E5, Moves::CAPTURE_FLAG)));

        // Position 3
        board.load_position("4r1k1/pp3ppp/2pb1B2/3p1b2/3P4/1BN4P/PPP2PP1/4R1K1 b - - 0 17");
        Evaluation::Eval gain3 = Evaluation::PieceValues[ROOK];
        ASSERT_EQUALS(gain3, board.see(Move(SQ_E8, SQ_E1, Moves::CAPTURE_FLAG)));

        return true;
    }

}