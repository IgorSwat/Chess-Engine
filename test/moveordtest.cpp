#include "test.h"
#include "../src/engine/moveord.h"


namespace Testing {

    // --------------------------------------
    // Move ordering test - discrete ordering
    // --------------------------------------

    // Focuses on testing the Selector class and selection of moves with application of various strategies
    REGISTER_TEST(move_ordering_discrete_test)
    {
        Board board;
        board.load_position("2r2r2/p1q1nk1p/bpp2pp1/8/4P1N1/1NQ5/PP3PPP/3RR1K1 w - - 2 22");

        MoveOrdering::Selector selector(&board, MoveGeneration::CAPTURE);
        selector.strategy.add_rule(MoveGeneration::CAPTURE, [&board](const EMove& move) -> bool {
            return type_of(board.on(move.from())) == QUEEN && file_of(move.to()) == FILE_F;
        });
        selector.strategy.add_rule(MoveGeneration::CAPTURE, [&board](const EMove& move) -> bool {
            return type_of(board.on(move.from())) == QUEEN;
        });
        selector.strategy.add_rule(MoveGeneration::QUIET_CHECK, [&board](const EMove& move) -> bool {
            return board.see(move) >= 0;
        });
        selector.strategy.add_rule(MoveGeneration::QUIET, [&board](const EMove& move) -> bool {
            return move.is_double_pawn_push();
        });

        // has_next() is called after every next() invocation to test whether it is safe to use in between other Selector operations
        ASSERT_EQUALS(true, selector.has_next());
        ASSERT_EQUALS(Move(SQ_C3, SQ_F6, Moves::CAPTURE_FLAG), selector.next());
        ASSERT_EQUALS(true, selector.has_next());
        ASSERT_EQUALS(Move(SQ_C3, SQ_C6, Moves::CAPTURE_FLAG), selector.next());
        ASSERT_EQUALS(true, selector.has_next());
        ASSERT_EQUALS(Move(SQ_G4, SQ_F6, Moves::CAPTURE_FLAG), selector.next());
        ASSERT_EQUALS(true, selector.has_next());
        ASSERT_EQUALS(Move(SQ_G4, SQ_H6, Moves::QUIET_MOVE_FLAG), selector.next());
        ASSERT_EQUALS(true, selector.has_next());
        ASSERT_EQUALS(Move(SQ_G4, SQ_E5, Moves::QUIET_MOVE_FLAG), selector.next());
        ASSERT_EQUALS(true, selector.has_next());
        ASSERT_EQUALS(Move(SQ_C3, SQ_C4, Moves::QUIET_MOVE_FLAG), selector.next());
        ASSERT_EQUALS(true, selector.has_next());
        ASSERT_EQUALS(Move(SQ_A2, SQ_A4, Moves::DOUBLE_PAWN_PUSH_FLAG), selector.next());
        ASSERT_EQUALS(true, selector.has_next());
        ASSERT_EQUALS(Move(SQ_F2, SQ_F4, Moves::DOUBLE_PAWN_PUSH_FLAG), selector.next());
        ASSERT_EQUALS(true, selector.has_next());
        ASSERT_EQUALS(Move(SQ_H2, SQ_H4, Moves::DOUBLE_PAWN_PUSH_FLAG), selector.next());
        ASSERT_EQUALS(true, selector.has_next());
        ASSERT_EQUALS(Move(SQ_A2, SQ_A3, Moves::QUIET_MOVE_FLAG), selector.next());

        return true;
    }


    // ----------------------------------------
    // Move ordering test - continuous ordering
    // ----------------------------------------

    REGISTER_TEST(move_ordering_continuous_test)
    {
        Board board;
        board.load_position("6r1/8/3p1b2/2p5/4N3/6q1/5b2/1k5K w - - 0 1");

        MoveOrdering::Selector selector(&board, MoveGeneration::CAPTURE);

        MoveOrdering::sort(selector, [&board](const Move& move) -> int32_t {
            return board.see(move);
        }, Moves::Enhancement::PURE_SEE);

        ASSERT_EQUALS(true, selector.has_next());
        ASSERT_EQUALS(Move(SQ_E4, SQ_G3, Moves::CAPTURE_FLAG), selector.next());
        ASSERT_EQUALS(true, selector.has_next());
        ASSERT_EQUALS(Move(SQ_E4, SQ_F6, Moves::CAPTURE_FLAG), selector.next());
        ASSERT_EQUALS(true, selector.has_next());
        ASSERT_EQUALS(Move(SQ_E4, SQ_F2, Moves::CAPTURE_FLAG), selector.next());
        ASSERT_EQUALS(true, selector.has_next());
        ASSERT_EQUALS(Move(SQ_E4, SQ_C5, Moves::CAPTURE_FLAG), selector.next());
        ASSERT_EQUALS(true, selector.has_next());
        ASSERT_EQUALS(Move(SQ_E4, SQ_D6, Moves::CAPTURE_FLAG), selector.next());

        selector.next(MoveOrdering::Selector::PARTIAL_CASCADE);
        MoveOrdering::sort(selector, [&board](const Move& move) -> int32_t {
            return board.see(move);
        }, Moves::Enhancement::PURE_SEE);

        ASSERT_EQUALS(true, selector.has_next());
        ASSERT_EQUALS(Move(SQ_E4, SQ_D2, Moves::QUIET_MOVE_FLAG), selector.next());
        ASSERT_EQUALS(true, selector.has_next());
        ASSERT_EQUALS(Move(SQ_E4, SQ_C3, Moves::QUIET_MOVE_FLAG), selector.next());

        selector.next(MoveOrdering::Selector::PARTIAL_CASCADE);
        ASSERT_EQUALS(true, selector.has_next());
        ASSERT_EQUALS(Move(SQ_E4, SQ_G5, Moves::QUIET_MOVE_FLAG), selector.next());
        ASSERT_EQUALS(false, selector.has_next());

        return true;
    }

}