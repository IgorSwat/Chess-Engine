#pragma once

#include "../logic/boardConfig.h"


// -----------
// SEE methods
// -----------

namespace SEE {

	int16_t evaluate(BoardConfig* board, Square from, Square to, PieceType promotionType = PAWN);

	inline int16_t evaluate(BoardConfig* board, Move& move)
	{
		move.see = evaluate(board, move.from(), move.to(), move.isPromotion() ? move.promotionType() : PAWN);
		return move.see;
	}

	inline int16_t evaluate(BoardConfig* board, const Move& move)
	{
		return evaluate(board, move.from(), move.to(), move.isPromotion() ? move.promotionType() : PAWN);
	}
}