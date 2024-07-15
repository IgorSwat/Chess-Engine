#pragma once

#include "types.h"
#include <iostream>
#include <iomanip>


// -------------------------
// Bitwise flags and defines
// -------------------------

using Movemask = uint16_t;
constexpr int MOVEMASK_SIZE = 16;

constexpr Movemask SPECIAL1_FLAG = 0x1;
constexpr Movemask SPECIAL2_FLAG = 0x2;
constexpr Movemask CAPTURE_FLAG = 0x4;
constexpr Movemask EXTENDED_CAPTURE_FLAG = 0x4000;
constexpr Movemask PROMOTION_FLAG = 0x8;
constexpr Movemask EXTENDED_PROMOTION_FLAG = 0x8000;

constexpr Movemask QUIET_MOVE_FLAG = 0x0;
constexpr Movemask DOUBLE_PAWN_PUSH_FLAG = 0x1;
constexpr Movemask ENPASSANT_FLAG = 0x5;
constexpr Movemask KINGSIDE_CASTLE_FLAG = 0x2;
constexpr Movemask QUEENSIDE_CASTLE_FLAG = 0x3;
constexpr Movemask KNIGHT_PROMOTION_FLAG = 0x8;
constexpr Movemask BISHOP_PROMOTION_FLAG = 0x9;
constexpr Movemask ROOK_PROMOTION_FLAG = 0xa;
constexpr Movemask QUEEN_PROMOTION_FLAG = 0xb;


// ----------
// Move types
// ----------

enum MoveType
{
	NORMAL_MOVE = 0, PROMOTION, CASTLE, ENPASSANT
};

constexpr MoveType MOVES_BY_FLAG[16]{
	NORMAL_MOVE, NORMAL_MOVE, CASTLE, CASTLE, NORMAL_MOVE, ENPASSANT, NORMAL_MOVE, NORMAL_MOVE,
	PROMOTION, PROMOTION, PROMOTION, PROMOTION, PROMOTION, PROMOTION, PROMOTION, PROMOTION
};


// ----------
// Move class
// ----------

// WARNING! Move class makes an assumption, that any Square ever given to it's methods will not be equal to INVALID_SQUARE.

class Move
{
public:
	constexpr Move() = default;
	Move(Square from, Square to, Movemask flags);
	Move(const Move& other);
	void operator=(const Move& other);

	Square from() const;
	Square to() const;
	Movemask butterflyIndex() const;
	Movemask flags() const;
	PieceType promotionType() const;

	void setFromSquare(Square from);
	void setToSquare(Square to);

	MoveType type() const;
	bool isCapture() const;
	bool isPromotion() const;
	bool isDoublePawnPush() const;
	bool isEnpassant() const;
	bool isCastle() const;

	static constexpr Move null();
	friend bool operator==(const Move& m1, const Move& m2);
	friend bool operator!=(const Move& m1, const Move& m2);
	friend std::ostream& operator<<(std::ostream& os, const Move& move);

private:
	Movemask m_move = 0;
};


// --------------
// Initialization
// --------------

inline Move::Move(Square from, Square to, Movemask flags) 
{
	m_move = ((flags & 0xf) << 12) | (static_cast<Movemask>(from) | (static_cast<Movemask>(to) << 6));
}

inline Move::Move(const Move& other)
{
	m_move = other.m_move;
}

inline void Move::operator=(const Move& other)
{
	m_move = other.m_move;
}


// -------
// Getters
// -------

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


// --------------
// Common setters
// --------------

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


// ------------------
// Move type checking
// ------------------

inline MoveType Move::type() const
{
	return MOVES_BY_FLAG[flags()];
}

inline bool Move::isCapture() const
{
	return m_move & EXTENDED_CAPTURE_FLAG;
}

inline bool Move::isPromotion() const
{
	return m_move & EXTENDED_PROMOTION_FLAG;
}

inline bool Move::isDoublePawnPush() const
{
	return flags() == DOUBLE_PAWN_PUSH_FLAG;
}

inline bool Move::isEnpassant() const
{
	return flags() == ENPASSANT_FLAG;
}

inline bool Move::isCastle() const
{
	Movemask mask = flags();
	return mask == KINGSIDE_CASTLE_FLAG || mask == QUEENSIDE_CASTLE_FLAG;
}


// -------------
// Miscellaneous
// -------------

constexpr inline Move Move::null()
{
	return Move();
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
	os << std::dec;
	os << "Move   |   from: " << char('A' + file_of(move.from())) << 1 + rank_of(move.from()) << " ";
	os << "to: " << char('A' + file_of(move.to())) << 1 + rank_of(move.to()) << " ";
	os << "flags: " << std::hex << move.flags();
	return os;
}