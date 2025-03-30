#include "test.h"
#include "../src/engine/pieces.h"


namespace Testing {

    // Test standard attacks
    // - Testing queen attacks covers both bishop and rook attacks
    REGISTER_TEST(sliding_piece_attack_test)
    {
        // (square, piece-placement-bitboard, expected-result), tested for Queen
		auto test_cases = {
			std::make_tuple(SQ_E4, 0x011000008a101010, 0x01925438e8384482ULL),
			std::make_tuple(SQ_A1, 0x0040200000040080, 0x01010101010503feULL),
			std::make_tuple(SQ_C8, 0x0a04010000000000, 0x0a0e112040800000ULL),
			std::make_tuple(SQ_C6, 0x000a0a0e00000000, 0x040e0a0e00000000ULL)
		};

        for (const auto& tcase : test_cases)
            ASSERT_EQUALS(std::get<2>(tcase), Pieces::piece_attacks_s<QUEEN>(std::get<0>(tcase), std::get<1>(tcase)));
        
        return true;
    }

    // Test x-ray attacks
    REGISTER_TEST(xray_attack_test)
    {
        // Rook test cases
		ASSERT_EQUALS(0x0000100000030000, Pieces::xray_attacks<ROOK>(SQ_E3, 0x0000101000350000, 0x0000001000140000));
		ASSERT_EQUALS(0x00010101010100fc, Pieces::xray_attacks<ROOK>(SQ_A1, 0x0001000008021303, 0x0000000008021303));
        
		// Bishop test cases
		ASSERT_EQUALS(0x0000000008102040, Pieces::xray_attacks<BISHOP>(SQ_B6, 0x0808060441000000, 0x0800020400000000));
		ASSERT_EQUALS(0x0000000000040000, Pieces::xray_attacks<BISHOP>(SQ_H8, 0x8000000008040000, 0x8000000008040000));

        return true;
    }

}