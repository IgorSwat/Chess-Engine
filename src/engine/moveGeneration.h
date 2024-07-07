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
	Move* begin();
	const Move* begin() const;
	Move* end();
	const Move* end() const;

	void push_back(const Move& move);
	void pop_back();
	void clear();
	void setEnd(Move* end);
	std::size_t size() const;

	friend std::ostream& operator<<(std::ostream& os, const MoveList& moveList);

	// Max possible number of different moves in a position
	static constexpr int MAX_MOVES = 256;

private:
	Move generatedMoves[MAX_MOVES] = {};
	Move* endPtr = generatedMoves;
};


// -----------------
// STL compatibility
// -----------------

inline Move* MoveList::begin()
{
	return generatedMoves;
}

inline const Move* MoveList::begin() const
{
	return generatedMoves;
}

inline Move* MoveList::end()
{
	return endPtr;
}

inline const Move* MoveList::end() const
{
	return endPtr;
}


// -------------------
// Vector-like methods
// -------------------

inline void MoveList::push_back(const Move& move)
{
	assert(endPtr != generatedMoves + MAX_MOVES);	// TODO: remove after testing
	*endPtr = move;
	endPtr++;
}

inline void MoveList::pop_back()
{
	assert(endPtr != generatedMoves);			   // TO DO: remove after testing
	endPtr--;
}

// It does not remove the elements, but rather sets end pointer to beginning for efficiency reasons
inline void MoveList::clear()
{
	endPtr = generatedMoves;
}

// Used mainly for STL algorithms, which cannot directly remove the elements or manipulate the range
inline void MoveList::setEnd(Move* end)
{
	endPtr = end;
}

inline std::size_t MoveList::size() const
{
	return endPtr - generatedMoves;
}


// -----------------------
// Move generation library
// -----------------------

namespace MoveGeneration {

	enum MoveGenType {
		QUIET = 0,		// Not a capture/promotion, and not a direct check (but could be a quiet discovered check)
		QUIET_CHECK,	// Not a capture/promotion, but a direct check instead
		CAPTURE,		// A capture or pawn promotion, since both affect the general material balance
		CHECK_EVASION,
		LEGAL, 			// Collective generation of legal moves for testing purposes
		PSEUDO_LEGAL	// Collective generation of pseudo legal moves for either testing or search
	};


	template <MoveGenType gen>
	void generate_moves(const BoardConfig& board, MoveList& moveList);

}