#ifndef MOVE_H
#define MOVE_H

#include "piece.h"
#include "movemask.h"
#include <string>

class Move
{
public:
    const PieceType piece;
    const Side color;
    const int pieceID;
    const Square targetPos;
    const int moveNumber;
    const PieceType promotionFlag;
    const MoveMask specialFlag;
    const float evaluation;
    Move() : piece(PieceType::KING), color(WHITE), pieceID(0), targetPos(Square(0, 0)), moveNumber(1),
        promotionFlag(PieceType::PAWN), specialFlag(MoveType::COMMON), evaluation(0.f) {}
    Move(PieceType movingPiece, Side c, int id, const Square& pos, int moveno, PieceType promotionf,
         MoveMask special = MoveMask(MoveType::COMMON), float eval = 0.f)
        : piece(movingPiece), color(c), pieceID(id), targetPos(pos), moveNumber(moveno), promotionFlag(promotionf),
          specialFlag(special), evaluation(eval) {}
    Move(Piece* movingPiece, const Square& pos, int moveno, PieceType promotionf,
         MoveMask special = MoveMask(MoveType::COMMON), float eval = 0.f)
        : Move(movingPiece->getType(), movingPiece->getColor(), movingPiece->getID(), pos, moveno, promotionf, special, eval) {}
    Move(Piece* movedPiece, bool capturef, bool checkf, int moveno, PieceType promotionf,
         MoveMask special = MoveMask(MoveType::COMMON), float eval = 0.f)
        : Move(movedPiece->getType(), movedPiece->getColor(), movedPiece->getID(),
               movedPiece->getPos(), moveno , promotionf, special, eval) {}
    Move(const Move& other) : piece(other.piece), color(other.color), pieceID(other.pieceID), targetPos(other.targetPos),
        moveNumber(other.moveNumber), promotionFlag(other.promotionFlag), specialFlag(other.specialFlag),
        evaluation(other.evaluation) {}
    std::string writeNotation() const;
    std::string writeEvaluation() const;
};

class Move2     // Lightweight version of Move class defined above
{
public:
    const int pieceID;
    const PieceType pieceType;
    const Square targetPos;
    const MoveMask specialFlag;
    const PieceType promotionFlag;
    Move2(int id, PieceType type, const Square& pos, MoveMask special = MoveMask(MoveType::COMMON),
          PieceType promotionf = PieceType::PAWN) : pieceID(id), pieceType(type), targetPos(pos),
          specialFlag(special), promotionFlag(promotionf) {}
    Move2(const Move2& other) : pieceID(other.pieceID), pieceType(other.pieceType), targetPos(other.targetPos),
          specialFlag(other.specialFlag), promotionFlag(other.promotionFlag) {}
};

#endif // MOVE_H
