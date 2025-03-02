#include "test.h"
#include "../src/engine/bitboards.h"


namespace Testing {

    // NOTE: We do not test basic algorithms like popcount or lsb, since their implementation
    //       is relatively simple and very rarely changes

    // Test shifts
    REGISTER_TEST(bitboards_shift_test)
    {
        const Bitboard diagonal = 0x8040201008040201;

        // Test correctness of static shift
        ASSERT_EQUALS(0x4020100804020100, Bitboards::shift_s<NORTH>(diagonal));
        ASSERT_EQUALS(0x8040201008040200, Bitboards::shift_s<NORTH_EAST>(diagonal));
        ASSERT_EQUALS(0x0000804020100804, Bitboards::shift_s<SOUTH_EAST>(diagonal));

        // Now if we know that static shift is fine, we can compare dynamic shift to static shift
        ASSERT_EQUALS(Bitboards::shift_s<NORTH>(diagonal), Bitboards::shift_d(diagonal, NORTH));
        ASSERT_EQUALS(Bitboards::shift_s<NORTH_EAST>(diagonal), Bitboards::shift_d(diagonal, NORTH_EAST));
        ASSERT_EQUALS(Bitboards::shift_s<NORTH_WEST>(diagonal), Bitboards::shift_d(diagonal, NORTH_WEST));
        ASSERT_EQUALS(Bitboards::shift_s<SOUTH>(diagonal), Bitboards::shift_d(diagonal, SOUTH));

        return true;
    }

    // Test fills
    REGISTER_TEST(bitboards_fill_test)
    {
        const Bitboard b1 = square_to_bb(SQ_E4);
        const Bitboard b2 = square_to_bb(SQ_E4) | square_to_bb(SQ_E5);

        // Vertical fills
        ASSERT_EQUALS(0x1010101010000000, Bitboards::fill<NORTH>(b1));
        ASSERT_EQUALS(0x1010101010000000, Bitboards::fill<NORTH>(b2));
        ASSERT_EQUALS(0x0000000010101010, Bitboards::fill<SOUTH>(b1));
        ASSERT_EQUALS(0x0000001010101010, Bitboards::fill<SOUTH>(b2));

        // Other directions
        ASSERT_EQUALS(0x00000010180c0603, Bitboards::fill<SOUTH_WEST>(b2));
        ASSERT_EQUALS(0x000000f0f0000000, Bitboards::fill<EAST>(b2));

        return true;
    }

}