#include "test.h"
#include "../engine/see.h"


namespace Testing {

    void seeTest()
	{
		BoardConfig board;

		const auto testCases = { std::make_tuple("1k1r4/1pp4p/p7/4p3/8/P5P1/1PP4P/2K1R3 w - -", Move(SQ_E1, SQ_E5, CAPTURE_FLAG), 125),
								 std::make_tuple("1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - -", Move(SQ_D3, SQ_E5, CAPTURE_FLAG), -313),
								 std::make_tuple("4r1k1/pp3ppp/2pb1B2/3p1b2/3P4/1BN4P/PPP2PP1/4R1K1 b - - 0 17", Move(SQ_E8, SQ_E1, CAPTURE_FLAG), 619) };

		int i = 0;
		for (const auto& testCase : testCases) {
			board.loadPosition(std::get<0>(testCase));
			int eval = SEE::evaluate(&board, std::get<1>(testCase));
			assert(eval == std::get<2>(testCase));
			std::cout << "Test " << i++ << " succeeded!\n";
		}
	}

}