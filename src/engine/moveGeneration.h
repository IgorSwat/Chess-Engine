#pragma once

#include "../logic/boardConfig.h"


// ---------------
// Move list class
// ---------------

// A lightweight, vector-like container used to iterate over given (dynamic) set of moves
// It's size is defined by a constant, which reduces complexity connected with dynamic memory management
class MoveList
{
public:
	// STL algorithms compatibility
	EnhancedMove* begin() { return generatedMoves; }
	const EnhancedMove* begin() const { return generatedMoves; }
	EnhancedMove* end() { return endPtr; }
	const EnhancedMove* end() const { return endPtr; }

	void push_back(const EnhancedMove& move) { *endPtr = move; endPtr++; }
	void pop_back() { endPtr--; }
	void clear() { endPtr = generatedMoves; }	// Does not remove elements directly, just manipulates with range pointers
	void setEnd(EnhancedMove* end) { endPtr = end; }
	std::size_t size() const { return endPtr - generatedMoves; }

	friend std::ostream& operator<<(std::ostream& os, const MoveList& moveList);

	// Upper bound number of different moves in a position
	static constexpr int MAX_SIZE = 256;

private:
	EnhancedMove generatedMoves[MAX_SIZE] = {};
	EnhancedMove* endPtr = generatedMoves;
};


// -----------------------
// Move generation library
// -----------------------

namespace MoveGeneration {

	enum MoveGenType {
		QUIET = 1,		// Not a capture/promotion, and not a direct check (but could be a quiet discovered check)
		QUIET_CHECK,	// Not a capture/promotion, but a direct check instead
		CAPTURE,		// A capture or pawn promotion, since both affect the general material balance
		CHECK_EVASION,
		
		LEGAL, 			// Collective generation of legal moves for testing purposes
		PSEUDO_LEGAL,	// Collective generation of pseudo legal moves for either testing or search

		NONE = 0
	};


	// Main move generator
	template <MoveGenType gen>
	void generate_moves(const BoardConfig& board, MoveList& moveList);

	// Other generators for non-engine stuff (GUI, tests, etc)
	// -------------------------------------------------------
	// Creates a move that needs to be checked by legalityTestFull()
	Move create_pseudo_move(const BoardConfig& board, Square from, Square to);

}