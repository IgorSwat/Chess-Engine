#pragma once

#include "../logic/boardConfig.h"


// -----------
// SEE methods
// -----------

namespace SEE {

	int evaluate(BoardConfig* board, Square from, Square to, PieceType promotionType = PAWN);

	inline int evaluate(BoardConfig* board, const Move& move)
	{
		return evaluate(board, move.from(), move.to(), move.isPromotion() ? move.promotionType() : PAWN);
	}
}