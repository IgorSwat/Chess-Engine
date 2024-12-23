#pragma once

#include "../logic/boardConfig.h"
#include "../utilities/lightList.h"


// ----------------
// Move list define
// ----------------

using MoveList = LightList<EnhancedMove, 256>;		// 256 is an upper bound maximum number of moves in any position


// -----------------------
// Move generation library
// -----------------------

namespace MoveGeneration {

	// Move generation phase definition
	enum Phase {
		QUIET = 1,		// Not a capture/promotion, and not a direct check (but could be a quiet discovered check)
		QUIET_CHECK,	// Not a capture/promotion, but a direct check instead
		CAPTURE,		// A capture or pawn promotion, since both affect the general material balance
		CHECK_EVASION,
		
		LEGAL, 			// Collective generation of legal moves for testing purposes
		PSEUDO_LEGAL,	// Collective generation of pseudo legal moves for either testing or search

		NONE = 0
	};

	// Main move generators
	// --------------------

	template <Phase gen>
	void generate_moves(const BoardConfig& board, MoveList& moveList);

	// Other generators for non-engine stuff (GUI, tests, etc)
	// -------------------------------------------------------

	// Minimalistic move creator - creates a move that needs to be checked by legalityTestFull()
	Move create_pseudo_move(const BoardConfig& board, Square from, Square to);

}