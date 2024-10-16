#pragma once

#include "../logic/boardConfig.h"


namespace SEE {

	// -----------------
	// Piece type values
	// -----------------

	constexpr int16_t PieceExchangeValue[PIECE_TYPE_RANGE] = { 
		0, 	// Covers the NULL_TYPE (= no piece on target square)
		125, 
		438, 
		438, 
		619, 
		1300, 
		20000, 
		0 
	};


	// -----------
	// SEE methods
	// -----------

	int16_t evaluate(BoardConfig* board, Square from, Square to, PieceType promotionType = PAWN);

	inline int16_t evaluate(BoardConfig* board, Move& move)
	{
		if (move == Move::null())
			return 0;

		if (move.see == NO_SEE)
			move.see = evaluate(board, move.from(), move.to(), move.isPromotion() ? move.promotionType() : PAWN);

		return move.see;
	}

	inline int16_t evaluate(BoardConfig* board, const Move& move)
	{
		return move == Move::null() ? 0 :
			   move.see == NO_SEE ? evaluate(board, move.from(), move.to(), move.isPromotion() ? move.promotionType() : PAWN) :
								    move.see;
	}
}