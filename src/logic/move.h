#pragma once

#include "misc.h"
#include <iostream>


using Movemask = uint16_t;
constexpr int MOVEMASK_SIZE = 16;

constexpr Movemask SPECIAL1_FLAG = 0x1;
constexpr Movemask SPECIAL2_FLAG = 0x2;
constexpr Movemask CAPTURE_FLAG = 0x4;
constexpr Movemask EXTENDED_CAPTURE_FLAG = 0x4000;
constexpr Movemask PROMOTION_FLAG = 0x8;
constexpr Movemask EXTENDED_PROMOTION_FLAG = 0x8000;

constexpr Movemask QUIET_MOVE = 0x0;
constexpr Movemask DOULBLE_PAWN_PUSH_FLAG = 0x1;
constexpr Movemask ENPASSANT_FLAG = 0x5;
constexpr Movemask KINGSIDE_CASTLE_FLAG = 0x2;
constexpr Movemask QUEENSIDE_CASTLE_FLAG = 0x3;
constexpr Movemask KNIGHT_PROMOTION_FLAG = 0x8;
constexpr Movemask BISHOP_PROMOTION_FLAG = 0x9;
constexpr Movemask ROOK_PROMOTION_FLAG = 0xa;
constexpr Movemask QUEEN_PROMOTION_FLAG = 0xb;


// WARNING! 
// Move class makes an assumption, that any Square ever given to it's methods will not be equal to INVALID_SQUARE.

class Move
{
public:
	Move(Square from, Square to, Movemask flags);
	void operator=(const Move& other);

	Square from() const;
	Square to() const;
	Movemask butterflyIndex() const;
	Movemask flags() const;
	PieceType promotionType() const;

	void setFromSquare(Square from);
	void setToSquare(Square to);

	bool isCapture() const;
	bool isDoublePawnPush() const;

	friend bool operator==(const Move& m1, const Move& m2);
	friend bool operator!=(const Move& m1, const Move& m2);
	friend std::ostream& operator<<(std::ostream& os, const Move& move);

private:
	Movemask m_move = 0;
};



inline Move::Move(Square from, Square to, Movemask flags) {
	m_move = ((flags & 0xf) << 12) | (static_cast<Movemask>(from) | (static_cast<Movemask>(to) << 6));
}

inline void Move::operator=(const Move& other)
{
	m_move = other.m_move;
}

inline Square Move::from() const
{
	return Square(m_move & 0x3f);
}

inline Square Move::to() const
{
	return Square((m_move >> 6) & 0x3f);
}

inline Movemask Move::butterflyIndex() const
{
	return m_move & 0x0fff;
}

inline Movemask Move::flags() const
{
	return m_move >> 12;
}

inline PieceType Move::promotionType() const
{
	return PieceType((flags() & 0x3) + 2);
}

inline void Move::setFromSquare(Square from)
{
	m_move &= ~0x3f;
	m_move |= static_cast<Movemask>(from);
}

inline void Move::setToSquare(Square to)
{
	m_move &= ~0xfc0;
	m_move |= (static_cast<Movemask>(to) << 6);
}

inline bool Move::isCapture() const
{
	return (m_move & EXTENDED_CAPTURE_FLAG) != 0;
}

inline bool Move::isDoublePawnPush() const
{
	return flags() == DOULBLE_PAWN_PUSH_FLAG;
}

inline bool operator==(const Move & m1, const Move & m2)
{
	return m1.m_move == m2.m_move;
}

inline bool operator!=(const Move& m1, const Move& m2)
{
	return m1.m_move != m2.m_move;
}

inline std::ostream& operator<<(std::ostream& os, const Move& move)
{
	os << move.m_move;
	return os;
}