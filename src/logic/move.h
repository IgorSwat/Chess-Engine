#pragma once

#include "types.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <numeric>


// -------------------------
// Bitwise flags and defines
// -------------------------

using Movemask = uint16_t;

constexpr int MOVEMASK_SIZE = 16;

constexpr Movemask SPECIAL1_FLAG = 0x1;
constexpr Movemask SPECIAL2_FLAG = 0x2;
constexpr Movemask CAPTURE_FLAG = 0x4;
constexpr Movemask PROMOTION_FLAG = 0x8;
constexpr Movemask EXTENDED_CAPTURE_FLAG = 0x4000;
constexpr Movemask EXTENDED_PROMOTION_FLAG = 0x8000;
constexpr Movemask NON_QUIET_MASK = EXTENDED_CAPTURE_FLAG | EXTENDED_PROMOTION_FLAG;

constexpr Movemask QUIET_MOVE_FLAG = 0x0;
constexpr Movemask DOUBLE_PAWN_PUSH_FLAG = 0x1;
constexpr Movemask ENPASSANT_FLAG = 0x5;
constexpr Movemask KINGSIDE_CASTLE_FLAG = 0x2;
constexpr Movemask QUEENSIDE_CASTLE_FLAG = 0x3;
constexpr Movemask KNIGHT_PROMOTION_FLAG = 0x8;
constexpr Movemask BISHOP_PROMOTION_FLAG = 0x9;
constexpr Movemask ROOK_PROMOTION_FLAG = 0xa;
constexpr Movemask QUEEN_PROMOTION_FLAG = 0xb;

constexpr int16_t NO_SEE = -16000;


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
	Move(Square from, Square to, Movemask flags)
		: m_move(((flags & 0xf) << 12) | (static_cast<Movemask>(from) | (static_cast<Movemask>(to) << 6))) {}

	Square from() const { return Square(m_move & 0x3f); }
	Square to() const { return Square((m_move >> 6) & 0x3f); }
	Movemask butterflyIndex() const { return m_move & 0x0fff; }
	Movemask flags() const { return m_move >> 12; }
	Movemask raw() const { return m_move; }
	PieceType promotionType() const { return PieceType((flags() & 0x3) + 2); }

	void setFromSquare(Square from) { m_move &= ~0x3f; m_move |= static_cast<Movemask>(from); }
	void setToSquare(Square to) { m_move &= ~0xfc0; m_move |= (static_cast<Movemask>(to) << 6); }

	MoveType type() const { return MOVES_BY_FLAG[flags()]; }
	bool isCapture() const { return m_move & EXTENDED_CAPTURE_FLAG; }
	bool isPromotion() const { return m_move & EXTENDED_PROMOTION_FLAG; }
	bool isQuiet() const { return !(m_move & NON_QUIET_MASK); }
	bool isDoublePawnPush() const { return flags() == DOUBLE_PAWN_PUSH_FLAG; }
	bool isEnpassant() const { return flags() == ENPASSANT_FLAG; }
	bool isCastle() const { return flags() == KINGSIDE_CASTLE_FLAG || flags() == QUEENSIDE_CASTLE_FLAG; }

	static constexpr Move null() { return Move(); }

	friend bool operator==(const Move& m1, const Move& m2) { return m1.m_move == m2.m_move; }
	friend bool operator!=(const Move& m1, const Move& m2) { return m1.m_move != m2.m_move; }
	friend std::ostream& operator<<(std::ostream& os, const Move& move);

protected:
	Movemask m_move = 0;
};


// ------------
// Move methods
// ------------

inline std::ostream& operator<<(std::ostream& os, const Move& move)
{
	os << std::dec;
	os << "Move   |   from: " << char('A' + file_of(move.from())) << 1 + rank_of(move.from()) << " ";
	os << "to: " << char('A' + file_of(move.to())) << 1 + rank_of(move.to()) << " ";
	os << "flags: " << std::hex << move.flags();
	return os;
}


// ------------------
// Extended move type
// ------------------

// A lightweight enum flag type
enum class EnhancementMode : uint8_t {
	PURE_SEE = 1,
	CUSTOM_SORTING = 2,

	NONE = 0
};

// Decorator pattern: move + additional info about the move
// Main purpose is to enable sorting MoveList in place
class EnhancedMove : public Move
{
public:
	EnhancedMove() = default;
	EnhancedMove(Square from, Square to, Movemask flags) : Move(from, to, flags) {}
	EnhancedMove(const Move& move) : Move(move) {}

	EnhancedMove& operator=(const Move& other) { m_move = other.raw(); return *this; }

	// A convenient setter which makes sure that index value is not just a magic number
	void enhance(EnhancementMode mode, int32_t key) { this->mode = mode; this->index = key; }

	int32_t key() const { return index; }
	int16_t see() const { return mode == EnhancementMode::PURE_SEE ? static_cast<int16_t>(index) : NO_SEE; }

private:
	EnhancementMode mode = EnhancementMode::NONE;
	int32_t index = 0;		// Sorting key - could be SEE or something else, depending on specified flag
};