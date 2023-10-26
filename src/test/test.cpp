#include "test.h"
#include <cassert>
#include <iostream>
using namespace std;

namespace Testing {
	void testMagics()
	{
		cout << Bitboards::bitboardToString(Pieces::queenAttacks(SQ_E4, 0x011000008a101010)) << endl << endl;
		assert(Pieces::queenAttacks(SQ_E4, 0x011000008a101010) == 0x01925438e8384482);
		cout << Bitboards::bitboardToString(Pieces::queenAttacks(SQ_A1, 0x0040200000040080)) << endl << endl;
		assert(Pieces::queenAttacks(SQ_A1, 0x0040200000040080) == 0x01010101010503fe);
		cout << Bitboards::bitboardToString(Pieces::queenAttacks(SQ_C8, 0x0a04010000000000)) << endl << endl;
		assert(Pieces::queenAttacks(SQ_C8, 0x0a04010000000000) == 0x0a0e112040800000);
		cout << Bitboards::bitboardToString(Pieces::queenAttacks(SQ_C6, 0x000a0a0e00000000)) << endl << endl;
		assert(Pieces::queenAttacks(SQ_C6, 0x000a0a0e00000000) == 0x040e0a0e00000000);
	}

	void testXRayAttacks()
	{
		assert(Pieces::xRayRookAttacks(SQ_E3, 0x0000101000350000, 0x0000001000140000) == 0x0000100000030000);
		assert(Pieces::xRayRookAttacks(SQ_A1, 0x0001000008021303, 0x0000000008021303) == 0x00010101010100fc);
		assert(Pieces::xRayBishopAttacks(SQ_B6, 0x0808060441000000, 0x0800020400000000) == 0x0000000008102040);
		assert(Pieces::xRayBishopAttacks(SQ_H8, 0x8000000008040000, 0x8000000008040000) == 0x0000000000040000);
	}

	void pinsAndChecksBoardconfigTestRuySteinitz()
	{
		BoardConfig board;
		board.makeMove(Move(SQ_E2, SQ_E4, DOULBLE_PAWN_PUSH_FLAG));
		board.makeMove(Move(SQ_E7, SQ_E5, DOULBLE_PAWN_PUSH_FLAG));
		board.makeMove(Move(SQ_G1, SQ_F3, QUIET_MOVE));
		board.makeMove(Move(SQ_B8, SQ_C6, QUIET_MOVE));
		board.makeMove(Move(SQ_F1, SQ_B5, QUIET_MOVE));
		assert(board.pinnedPieces(WHITE) == 0);
		assert(board.pinnedPieces(BLACK) == 0);
		assert(board.checkingPieces() == 0);
		board.makeMove(Move(SQ_D7, SQ_D6, QUIET_MOVE));
		assert(board.pinnedPieces(BLACK) == squareToBB(SQ_C6));
		assert(board.pinningPieces(BLACK) == squareToBB(SQ_B5));
		board.makeMove(Move(SQ_E1, SQ_G1, KINGSIDE_CASTLE_FLAG));
		board.makeMove(Move(SQ_A7, SQ_A6, QUIET_MOVE));
		assert(board.checkingPieces() == 0);
		board.makeMove(Move(SQ_B5, SQ_C6, CAPTURE_FLAG));
		assert(board.checkingPieces() == squareToBB(SQ_C6));
		assert(board.pinnedPieces(WHITE) == 0);
		assert(board.pinnedPieces(BLACK) == 0);
		board.makeMove(Move(SQ_C8, SQ_D7, QUIET_MOVE));
		assert(board.checkingPieces() == 0);
		assert(board.pinnedPieces(BLACK) == squareToBB(SQ_D7));
		assert(board.pinningPieces(BLACK) == squareToBB(SQ_C6));
		board.makeMove(Move(SQ_F1, SQ_E1, QUIET_MOVE));
		board.makeMove(Move(SQ_B7, SQ_C6, CAPTURE_FLAG));
		assert(board.checkingPieces() == 0);
		assert(board.pinnedPieces(WHITE) == 0);
		assert(board.pinningPieces(BLACK) == 0);
		assert(board.pinnedPieces(BLACK) == 0);
	}
}