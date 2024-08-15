#include "see.h"
#include <numeric>


namespace SEE {

	constexpr int MAX_DEPTH = 33;
	constexpr int16_t PieceExchangeValue[PIECE_TYPE_RANGE] = { 
		0, 	// Covers the NULL_TYPE (= no piece on target square)
		100, 
		325, 
		325, 
		500, 
		900, 
		16000, 
		0 
	};


	// ----------------
	// Helper functions
	// ----------------

	Bitboard lvp(BoardConfig* board, Color side, Bitboard area, PieceType &type)
	{
		for (type = PAWN; type <= KING; type = PieceType(type + 1))
		{
			Bitboard subset = area & board->pieces(side, type);
			if (subset)
				return square_to_bb(Bitboards::lsb(subset));
		}
		return 0;
	}


	// -----------
	// SEE methods
	// -----------

	int16_t evaluate(BoardConfig* board, Square from, Square to, PieceType promotionType)
	{
		// Check whether a real exchange happens (Possible to also check the color of both pieces)
		if (board->isFree(from) || board->isFree(to))
			return 0;

		int16_t gain[MAX_DEPTH];
		int depth = board->movingSide();
		Bitboard mayXray = board->pieces() ^ board->pieces(KNIGHT) ^ board->pieces(KING);
		Bitboard occ = board->pieces();
		Bitboard fromSet = square_to_bb(from);
		Bitboard attackdef = board->attackersToSquare(to, occ);
		PieceType attackedPiece = type_of(board->onSquare(to));
		PieceType attackingPiece = type_of(board->onSquare(from));

		gain[depth] = PieceExchangeValue[attackedPiece] + PieceExchangeValue[promotionType] - PieceExchangeValue[PAWN];
		do {
			depth++;
			gain[depth] = PieceExchangeValue[attackingPiece] - gain[depth - 1];
			if (std::max<int16_t>(-gain[depth - 1], gain[depth]) < 0)
				break;
			attackdef ^= fromSet;
			if (fromSet & mayXray)
			{
				Bitboard xRayAttackers = (Pieces::xray_attacks<BISHOP>(to, occ, fromSet) & board->pieces(BISHOP, QUEEN)) |
										 (Pieces::xray_attacks<ROOK>(to, occ, fromSet) & board->pieces(ROOK, QUEEN));
				attackdef |= xRayAttackers;
			}
			occ ^= fromSet;
			fromSet = lvp(board, Color(depth & 0x1), attackdef, attackingPiece);
		} while (fromSet);
		while (--depth)
			gain[depth - 1] = -std::max<int16_t>(-gain[depth - 1], gain[depth]);
		return gain[board->movingSide()];
	}
	
}