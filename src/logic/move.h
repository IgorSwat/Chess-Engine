#pragma once

#include "piece.h"
#include <string>
#include <bitset>
#include <optional>

class BoardConfig;
class Move;
class Move2;

namespace Moves {
    enum MoveFlags { COMMON_FLAG = 3, ATTACK_FLAG = 2, CASTLE_FLAG = 1, ENPASSANT_FLAG = 0};

    // Creates a dummy Move object, which is always correct when move itself is legal, and may or may not be correct if move is illegal
    std::optional<Move> createMove(const BoardConfig* config, const Square& oldPos, const Square& newPos);
    Move2 fillExtraMoveData(const Move& move, const BoardConfig* config, int eval = 0);
    std::string writeEvaluation(int eval);
}

class Move
{
public:
    Move() : pieceID(0), pieceType(KING), promotionType(PAWN), targetPos(Square(0, 0)), flags(0b0000) {}
    Move(int id, PieceType type, const Square& position, int bitFlags, PieceType promoteTo = PAWN)
        : pieceID(id), pieceType(type), promotionType(promoteTo), targetPos(position), flags(bitFlags) {}
    Move(const Move& other)
        : pieceID(other.pieceID), pieceType(other.pieceType), promotionType(other.promotionType), targetPos(other.targetPos), flags(other.flags) {}
    bool operator==(const Move& other) const;
    bool hasProperty(Moves::MoveFlags flag) const { return flags[flag]; }
    
    const int pieceID;
    const PieceType pieceType;
    const PieceType promotionType;
    const Square targetPos;
    const std::bitset<4> flags;
};

class Move2
{
public:
    Move2() = default;
    Move2(const Move& move, const Square& oldPos, int moveNumber, bool captureFlag, int eval = 0)
        : move(move), oldPos(oldPos), moveNumber(moveNumber), captureFlag(captureFlag), eval(eval) {}
    std::string getMoveNotation() const;

    Move move;
    Square oldPos;
    int moveNumber;
    bool captureFlag;
    int eval;
};