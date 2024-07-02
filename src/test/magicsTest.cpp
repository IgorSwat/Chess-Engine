#include "test.h"
#include "../logic/pieces.h"
#include <iostream>


namespace Testing {

    void magicsTest()
	{
		// (square, piece-placement-bitboard, expected-result), tested for Queen
		const auto testCases = {
			std::make_tuple(SQ_E4, 0x011000008a101010, 0x01925438e8384482ULL),
			std::make_tuple(SQ_A1, 0x0040200000040080, 0x01010101010503feULL),
			std::make_tuple(SQ_C8, 0x0a04010000000000, 0x0a0e112040800000ULL),
			std::make_tuple(SQ_C6, 0x000a0a0e00000000, 0x040e0a0e00000000ULL)
		};

		for (const auto& testCase : testCases) {
			assert(Pieces::piece_attacks_s<QUEEN>(std::get<0>(testCase), std::get<1>(testCase)) == std::get<2>(testCase));
		}
	}

	void xRayAttacksTest()
	{
		// Rook test cases
		assert(Pieces::xray_attacks<ROOK>(SQ_E3, 0x0000101000350000, 0x0000001000140000) == 0x0000100000030000);
		assert(Pieces::xray_attacks<ROOK>(SQ_A1, 0x0001000008021303, 0x0000000008021303) == 0x00010101010100fc);
		// Bishop test cases
		assert(Pieces::xray_attacks<BISHOP>(SQ_B6, 0x0808060441000000, 0x0800020400000000) == 0x0000000008102040);
		assert(Pieces::xray_attacks<BISHOP>(SQ_H8, 0x8000000008040000, 0x8000000008040000) == 0x0000000000040000);
	}

}