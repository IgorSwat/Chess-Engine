#pragma once

#include "../logic/boardConfig.h"


class MoveList
{
public:
	static constexpr int MAX_MOVES = 256;

	// STL algorithms compatibility
	Move* begin();
	const Move* begin() const;
	Move* end();
	const Move* end() const;

	void push_back(const Move& move);
	void pop_back();
	void clear();
	std::size_t size() const;
	void setEnd(Move* end);

	friend std::ostream& operator<<(std::ostream& os, const MoveList& moveList);

private:
	Move generatedMoves[MAX_MOVES] = {};
	Move* endPtr = generatedMoves;
};


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

inline void MoveList::push_back(const Move& move)
{
	assert(endPtr != generatedMoves + MAX_MOVES);	// TO DO: remove after testing
	*endPtr = move;
	endPtr++;
}

inline void MoveList::pop_back()
{
	assert(endPtr != generatedMoves);
	endPtr--;
}

inline void MoveList::clear()
{
	endPtr = generatedMoves;
}

inline std::size_t MoveList::size() const
{
	return endPtr - generatedMoves;
}

inline void MoveList::setEnd(Move* end)
{
	endPtr = end;
}



namespace MoveGeneration {

	enum MoveGenType {
		QUIET = 0,
		QUIET_CHECK,
		CAPTURE,
		CHECK_EVASION,
		LEGAL, 			// Collective generation of legal moves for testing purposes
		PSEUDO_LEGAL	// Collective generation of pseudo legal moves for either testing or search
	};


	template <MoveGenType gen>
	void generateMoves(MoveList& moveList, const BoardConfig& board);

	template <bool capture>
	inline void generatePromotions(MoveList& moveList, Square from, Square to)
	{
		constexpr Movemask queenPromoFlags = (capture ? CAPTURE_FLAG | QUEEN_PROMOTION_FLAG : QUEEN_PROMOTION_FLAG);
		constexpr Movemask rookPromoFlags = (capture ? CAPTURE_FLAG | ROOK_PROMOTION_FLAG : ROOK_PROMOTION_FLAG);
		constexpr Movemask bishopPromoFlags = (capture ? CAPTURE_FLAG | BISHOP_PROMOTION_FLAG : BISHOP_PROMOTION_FLAG);
		constexpr Movemask knightPromoFlags = (capture ? CAPTURE_FLAG | KNIGHT_PROMOTION_FLAG : KNIGHT_PROMOTION_FLAG);
		moveList.push_back(Move(from, to, queenPromoFlags));
		moveList.push_back(Move(from, to, rookPromoFlags));
		moveList.push_back(Move(from, to, bishopPromoFlags));
		moveList.push_back(Move(from, to, knightPromoFlags));
	}
}