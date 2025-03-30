#include "test.h"
#include "../src/engine/nnue.h"
#include <memory>
#include <vector>


// - NOTE: All tests here assume that we do not change network parameters (otherwise expected values might not be correct)
namespace Testing {

    // This test focuses on correctness of set() method, followed by forward pass
    REGISTER_TEST(nnue_static_load_test)
    {
        Board board;
        std::unique_ptr<Evaluation::NNUE> nnue = std::make_unique<Evaluation::NNUE>();

        // Load network parameters
        nnue->load("model/model_best.nnue");

        // Perform test on a few positions
        // - Each test is very simple in this case - set position on the board, use set() method, and check result from forward()
        std::vector<std::pair<std::string, int>> positions = {
            {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 14},
            {"8/1p4p1/2p1k2p/4b3/P3Kp1P/1P6/2PB2P1/8 b - - 1 33", -66},
            {"3rnrk1/1pp1bppp/2qp4/4P3/5B2/1QN5/PPP2PPP/R2R2K1 w - - 5 16", 188},
            {"r3kbnr/pppq1ppp/3p4/8/3QP3/2N5/PPP2PPP/R1B1K2R b KQkq - 0 8", -32},
            {"r1bq1rk1/1pp2p2/pn2p2p/n3P3/PbpPN3/5N2/1PQ1BPPP/R2R2K1 w - - 0 14", 59}
        };

        for (auto [fen, eval] : positions) {
            board.load_position(fen);
            nnue->set(board);

            ASSERT_EQUALS(eval, nnue->forward(board));
        }

        return true;
    }

    // This test focuses on correctness of dynamic update of NNUE, trying to simulate real search-tree situations
    REGISTER_TEST(nnue_dynamic_change_test)
    {
        Board board;
        std::unique_ptr<Evaluation::NNUE> nnue = std::make_unique<Evaluation::NNUE>();

        // Load network parameters
        nnue->load("model/model_best.nnue");

        // Start with some custom position when both sides can castle
        board.load_position("r3kbnr/pppq1ppp/2npb3/1B2p3/4P3/2NP1N2/PPP2PPP/R1BQK2R w KQkq - 3 6");
        nnue->set(board);

        ASSERT_EQUALS(61, nnue->forward(board));

        // Now, let's make some moves
        nnue->update(board, Move(SQ_E1, SQ_G1, Moves::KINGSIDE_CASTLE_FLAG));
        board.make_move(Move(SQ_E1, SQ_G1, Moves::KINGSIDE_CASTLE_FLAG));
        nnue->update(board, Move(SQ_E8, SQ_C8, Moves::QUEENSIDE_CASTLE_FLAG));
        board.make_move(Move(SQ_E8, SQ_C8, Moves::QUEENSIDE_CASTLE_FLAG));

        ASSERT_EQUALS(114, nnue->forward(board));

        nnue->update(board, Move(SQ_B5, SQ_C6, Moves::CAPTURE_FLAG));
        board.make_move(Move(SQ_B5, SQ_C6, Moves::CAPTURE_FLAG));
        nnue->update(board, Move(SQ_D7, SQ_C6, Moves::CAPTURE_FLAG));
        board.make_move(Move(SQ_D7, SQ_C6, Moves::CAPTURE_FLAG));

        ASSERT_EQUALS(36, nnue->forward(board));

        nnue->update(board, Move(SQ_C1, SQ_E3, Moves::QUIET_MOVE_FLAG));
        board.make_move(Move(SQ_C1, SQ_E3, Moves::QUIET_MOVE_FLAG));

        ASSERT_EQUALS(-28, nnue->forward(board));

        nnue->undo_state();
        nnue->undo_state();
        nnue->undo_state();
        board.undo_move();
        board.undo_move();
        board.undo_move();

        ASSERT_EQUALS(114, nnue->forward(board));

        nnue->update(board, Move(SQ_B5, SQ_C6, Moves::CAPTURE_FLAG));
        board.make_move(Move(SQ_B5, SQ_C6, Moves::CAPTURE_FLAG));
        nnue->update(board, Move(SQ_D7, SQ_C6, Moves::CAPTURE_FLAG));
        board.make_move(Move(SQ_D7, SQ_C6, Moves::CAPTURE_FLAG));

        ASSERT_EQUALS(36, nnue->forward(board));

        // And let's load the initial position again
        board.load_position("r3kbnr/pppq1ppp/2npb3/1B2p3/4P3/2NP1N2/PPP2PPP/R1BQK2R w KQkq - 3 6");
        nnue->set(board);

        ASSERT_EQUALS(61, nnue->forward(board));

        // Load different position and test correctness in case of promotion or enpassant
        board.load_position("8/1k1pp1P1/2n5/5P2/8/8/8/1K4N1 w - - 0 1");
        nnue->set(board);

        nnue->update(board, Move(SQ_G7, SQ_G8, Moves::QUEEN_PROMOTION_FLAG));
        board.make_move(Move(SQ_G7, SQ_G8, Moves::QUEEN_PROMOTION_FLAG));

        ASSERT_EQUALS(-416, nnue->forward(board));

        nnue->undo_state();
        board.undo_move();

        ASSERT_EQUALS(293, nnue->forward(board));

        nnue->update(board, Move(SQ_G7, SQ_G8, Moves::KNIGHT_PROMOTION_FLAG));
        board.make_move(Move(SQ_G7, SQ_G8, Moves::KNIGHT_PROMOTION_FLAG));
        nnue->update(board, Move(SQ_E7, SQ_E5, Moves::DOUBLE_PAWN_PUSH_FLAG));
        board.make_move(Move(SQ_E7, SQ_E5, Moves::DOUBLE_PAWN_PUSH_FLAG));
        nnue->update(board, Move(SQ_F5, SQ_E6, Moves::ENPASSANT_FLAG));
        board.make_move(Move(SQ_F5, SQ_E6, Moves::ENPASSANT_FLAG));
        nnue->update(board, Move(SQ_D7, SQ_E6, Moves::CAPTURE_FLAG));
        board.make_move(Move(SQ_D7, SQ_E6, Moves::CAPTURE_FLAG));

        ASSERT_EQUALS(106, nnue->forward(board));

        return true;
    }

}