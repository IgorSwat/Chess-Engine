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

	void staticLoadingTest()
	{
		BoardConfig board;
		assert(board.pinnedPieces(WHITE) == 0);
		assert(board.pinnedPieces(BLACK) == 0);
		assert(board.checkingPieces() == 0);
		assert(board.enpassantSquare() == INVALID_SQUARE);
		assert(board.hasCastlingRight(ALL_RIGHTS));
		assert(board.pieces(WHITE) == 0x000000000000ffff);
		assert(board.pieces(BLACK) == 0xffff000000000000);

		board.loadFromFen("rnbqkb1r/1p2pp1p/2p2n2/1B1P2p1/pP6/7N/P1PPQPPP/RNB1KR2 b Qkq b3 0 7");
		assert(board.pinnedPieces(WHITE) == 0);
		assert(board.pinnedPieces(BLACK) == 0x0010040000000000);
		assert(board.pinningPieces(BLACK) == 0x0000000200001000);
		assert(board.enpassantSquare() == SQ_B4);
		assert(board.hasCastlingRight(BLACK_BOTH));
		assert(!board.hasCastlingRight(WHITE_OO));
		assert(board.hasCastlingRight(WHITE_OOO));
		assert(board.pieces(WHITE) == 0x0000000a0280fd37);
		assert(board.pieces(BLACK) == 0xbfb2244001000000);

		BoardConfig board2;
		board2.loadFromConfig(board);
		assert(board2.pinnedPieces(WHITE) == 0);
		assert(board2.pinnedPieces(BLACK) == 0x0010040000000000);
		assert(board2.pinningPieces(BLACK) == 0x0000000200001000);
		assert(board2.enpassantSquare() == SQ_B4);
		assert(board2.hasCastlingRight(BLACK_BOTH));
		assert(!board2.hasCastlingRight(WHITE_OO));
		assert(board2.hasCastlingRight(WHITE_OOO));
		assert(board2.pieces(WHITE) == 0x0000000a0280fd37);
		assert(board2.pieces(BLACK) == 0xbfb2244001000000);
	}

	void pinsAndChecksBoardconfigTestRuySteinitz()
	{
		BoardConfig board;
		board.makeMove(Move(SQ_E2, SQ_E4, DOUBLE_PAWN_PUSH_FLAG));
		board.makeMove(Move(SQ_E7, SQ_E5, DOUBLE_PAWN_PUSH_FLAG));
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

		board.undoLastMove();
		board.undoLastMove();
		assert(board.checkingPieces() == 0);
		assert(board.pinnedPieces(BLACK) == squareToBB(SQ_D7));
		assert(board.pinningPieces(BLACK) == squareToBB(SQ_C6));
		board.undoLastMove();
		assert(board.checkingPieces() == squareToBB(SQ_C6));
		assert(board.pinnedPieces(WHITE) == 0);
		assert(board.pinnedPieces(BLACK) == 0);
		board.undoLastMove();
		assert(board.checkingPieces() == 0);
		board.undoLastMove();
		board.undoLastMove();
		assert(board.pinnedPieces(BLACK) == squareToBB(SQ_C6));
		assert(board.pinningPieces(BLACK) == squareToBB(SQ_B5));
		board.undoLastMove();
		assert(board.pinnedPieces(WHITE) == 0);
		assert(board.pinnedPieces(BLACK) == 0);
		assert(board.checkingPieces() == 0);
		board.undoLastMove();
		board.undoLastMove();
		board.undoLastMove();
		board.undoLastMove();
		board.undoLastMove();
		assert(board.pinnedPieces(WHITE) == 0);
		assert(board.pinnedPieces(BLACK) == 0);
		assert(board.checkingPieces() == 0);
	}

	void castlingRightsAndEnPassantTest()
	{
		BoardConfig board;
		board.makeMove(Move(SQ_E2, SQ_E4, DOUBLE_PAWN_PUSH_FLAG));
		board.makeMove(Move(SQ_D7, SQ_D6, QUIET_MOVE));
		board.makeMove(Move(SQ_E4, SQ_E5, QUIET_MOVE));
		assert(board.enpassantSquare() == INVALID_SQUARE);
		board.makeMove(Move(SQ_F7, SQ_F5, DOUBLE_PAWN_PUSH_FLAG));
		assert(board.enpassantSquare() == SQ_F5);
		board.makeMove(Move(SQ_G1, SQ_F3, QUIET_MOVE));
		assert(board.enpassantSquare() == INVALID_SQUARE);
		board.makeMove(Move(SQ_D6, SQ_D5, QUIET_MOVE));
		board.makeMove(Move(SQ_F1, SQ_E2, QUIET_MOVE));
		board.makeMove(Move(SQ_G8, SQ_H6, QUIET_MOVE));
		assert(board.hasCastlingRight(ALL_RIGHTS));
		board.makeMove(Move(SQ_H1, SQ_G1, QUIET_MOVE));
		assert(!board.hasCastlingRight(WHITE_BOTH));
		assert(board.hasCastlingRight(WHITE_OOO));
		board.makeMove(Move(SQ_E7, SQ_E6, QUIET_MOVE));
		board.makeMove(Move(SQ_H2, SQ_H4, DOUBLE_PAWN_PUSH_FLAG));
		board.makeMove(Move(SQ_F8, SQ_B4, QUIET_MOVE));
		board.makeMove(Move(SQ_C2, SQ_C3, QUIET_MOVE));
		assert(board.hasCastlingRight(BLACK_BOTH));
		board.makeMove(Move(SQ_E8, SQ_G8, KINGSIDE_CASTLE_FLAG));
		assert(!board.hasCastlingRight(BLACK_OO));
		assert(!board.hasCastlingRight(BLACK_OOO));

		board.undoLastMove();
		assert(board.hasCastlingRight(BLACK_BOTH));
		board.undoLastMove();
		board.undoLastMove();
		board.undoLastMove();
		board.undoLastMove();
		assert(!board.hasCastlingRight(WHITE_BOTH));
		assert(board.hasCastlingRight(WHITE_OOO));
		board.undoLastMove();
		assert(board.hasCastlingRight(ALL_RIGHTS));
		board.undoLastMove();
		board.undoLastMove();
		board.undoLastMove();
		assert(board.enpassantSquare() == INVALID_SQUARE);
		board.undoLastMove();
		assert(board.enpassantSquare() == SQ_F5);
		board.undoLastMove();
		assert(board.enpassantSquare() == INVALID_SQUARE);
		board.undoLastMove();
		board.undoLastMove();
		board.undoLastMove();
		assert(board.enpassantSquare() == INVALID_SQUARE);
		assert(board.hasCastlingRight(ALL_RIGHTS));
	}
}