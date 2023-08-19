#include "move.h"
#include <sstream>
#include <iomanip>
using std::string;

string Move::writeNotation() const
{
    std::ostringstream outstr;
    outstr<<moveNumber<<".";
    bool pawnFlag = false;
    switch (piece)
    {
    case PieceType::KING:
        outstr<<"K";
        break;
    case PieceType::PAWN:
        outstr<<static_cast<char>('a' + targetPos.x);
        pawnFlag = true;
        break;
    case KNIGHT:
        outstr<<"N";
        break;
    case BISHOP:
        outstr<<"B";
        break;
    case ROOK:
        outstr<<"R";
        break;
    case QUEEN:
        outstr<<"Q";
        break;
    default:
        break;
    }
    if (!pawnFlag)
        outstr<<static_cast<char>('a' + targetPos.x);
    outstr<<(8 - targetPos.y);
    switch (promotionFlag)
    {
    case QUEEN:
        outstr<<"=Q";
        break;
    case ROOK:
        outstr<<"=R";
        break;
    case BISHOP:
        outstr<<"=B";
        break;
    case KNIGHT:
        outstr<<"=N";
        break;
    default:
        break;
    }
    return outstr.str();
}

string Move::writeEvaluation() const
{
    std::ostringstream outstr;
    if (evaluation >= 0.f)
        outstr<<"+";
    outstr<<std::fixed<<std::setprecision(1)<<evaluation;
    return outstr.str();
}

