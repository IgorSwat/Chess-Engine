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

	int16_t evaluate(const BoardConfig* board, Square from, Square to, PieceType promotionType = PAWN);

	inline int16_t evaluate(const BoardConfig* board, const Move& move)
	{
		return evaluate(board, move.from(), move.to(), move.isPromotion() ? move.promotionType() : PAWN);
	}

	inline int16_t evaluate(const BoardConfig* board, const EnhancedMove& move)
	{
		int16_t see = move.see();

		return see == NO_SEE ? evaluate(board, move.from(), move.to(), move.isPromotion() ? move.promotionType() : PAWN) :
							   see;
	}

	// Just as previous version, but this one modifies move index by setting it to calculated SEE value
	inline int16_t evaluate_save(const BoardConfig* board, EnhancedMove& move)
	{
		if (move.see() == NO_SEE)
			move.enhance(EnhancementMode::PURE_SEE, 
						 evaluate(board, move.from(), move.to(), move.isPromotion() ? move.promotionType() : PAWN));
		
		return move.see();
	}
}