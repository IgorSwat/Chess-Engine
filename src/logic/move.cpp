#include "move.h"
#include "boardconfig.h"
#include <sstream>
#include <iomanip>

namespace {
    const std::string pieceSymbols[6]{ "", "N", "B", "R", "Q", "K"};
    const std::string castlingSymbols[2]{ "O-O", "O-O-O" };
    const std::string columnSymbols[8]{ "a", "b", "c", "d", "e", "f", "g", "h" };
    constexpr int rowIndeces[8]{ 8, 7, 6, 5, 4, 3, 2, 1 };
}

namespace Moves {
    std::optional<Move> createMove(const BoardConfig* config, const Square& oldPos, const Square& newPos)
    {
        if (oldPos == newPos)
            return std::nullopt;
        const Piece* movingPiece = config->getPiece(oldPos);
        if (movingPiece == nullptr)
            return std::nullopt;
        const Piece* attackedPiece = config->getPiece(newPos);
        if (attackedPiece != nullptr && attackedPiece->getColor() == movingPiece->getColor())
            return std::nullopt;
        int bitmask = 0b1100;
        switch (movingPiece->getType())
        {
        case PAWN:
            if (oldPos.x == newPos.x)
                bitmask = 0b1000;
            else if (attackedPiece == nullptr)
                bitmask = 0b1001;
            break;
        case KING:
            if ((newPos.x - oldPos.x == 2 || newPos.x - oldPos.x == -2) && newPos.y == oldPos.y)
                bitmask = 0b1010;
            break;
        default:
            break;
        }
        return std::make_optional<Move>(movingPiece->getID(), movingPiece->getType(), newPos, bitmask);
    }

    Move2 fillExtraMoveData(const Move& move, const BoardConfig* config, int eval)
    {
        Side side = move.pieceID < 16 ? WHITE : BLACK;
        const Piece* onTargetSquare = config->getPiece(move.targetPos);
        int moveNumber = side == WHITE ? config->getMoveNumber() + 1 : config->getMoveNumber();
        bool captureFlag = move.hasProperty(ENPASSANT_FLAG) || (onTargetSquare != nullptr && onTargetSquare->getColor() != side);
        const Square& oldPos = config->getPiece(move.pieceID)->getPosition();
        return Move2(move, oldPos, moveNumber, captureFlag, eval);
    }

    std::string writeEvaluation(int eval)
    {
        std::ostringstream result;
        float evalNormalized = eval / 100.f;
        if (eval >= 0)
            result << "+";
        result << std::fixed << std::setprecision(1) << evalNormalized;
        return result.str();
    }
}



// Move class methods
bool Move::operator==(const Move& other) const
{
    return pieceID == other.pieceID && pieceType == other.pieceType && targetPos == other.targetPos && flags == other.flags;
}



// Move2 class methods
std::string Move2::getMoveNotation() const
{
    std::ostringstream notation;
    notation << moveNumber << ".";
    if (move.hasProperty(Moves::CASTLE_FLAG))
    {
        CastleType castleType = move.targetPos.x > oldPos.x ? SHORT_CASTLE : LONG_CASTLE;
        notation << castlingSymbols[castleType];
    }
    else if (move.pieceType == PAWN)
    {
        notation << columnSymbols[oldPos.x];
        if (captureFlag)
            notation << "x" << columnSymbols[move.targetPos.x];
        notation << rowIndeces[move.targetPos.y];
        if (move.promotionType != PAWN)
            notation << "=" << pieceSymbols[move.promotionType];
    }
    else
    {
        notation << pieceSymbols[move.pieceType];
        if (captureFlag)
            notation << "x";
        notation << columnSymbols[move.targetPos.x] << rowIndeces[move.targetPos.y];
    }
    return notation.str();
}
