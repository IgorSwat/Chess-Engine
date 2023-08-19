#include <cmath>
#include "piece.h"

dirVector Piece::pawnDirs({Square(0, 1)});
dirVector Piece::knightDirs({Square(1, -2), Square(2, -1), Square(2, 1), Square(1, 2), Square(-1, 2),
                                Square(-2, 1), Square(-2, -1), Square(-1, -2)});
dirVector Piece::bishopDirs({Square(1, -1), Square(1, 1), Square(-1, 1), Square(-1, -1)});
dirVector Piece::rookDirs({Square(0, -1), Square(1, 0), Square(0, 1), Square(-1, 0)});
dirVector Piece::queenDirs({Square(0, -1), Square(1, -1), Square(1, 0), Square(1, 1), Square(0, 1),
                                Square(-1, 1), Square(-1, 0), Square(-1, -1)});
dirVector King::initialSquares({Square(4, 7), Square(4, 0)});



Square Piece::getMovementVector(const Square& targetPos) const
{
    if (isReachable(targetPos))
    {
        Square longVector = targetPos - square;
        int xOffset, yOffset;
        if (longVector.x == 0)
            xOffset = 0;
        else
            xOffset = longVector.x / abs(longVector.x);
        if (longVector.y == 0)
            yOffset = 0;
        else
            yOffset = longVector.y / abs(longVector.y);
        return Square(xOffset, yOffset);
    }
    return Square(8, 8);
}

// Pawn methods
bool Pawn::isReachable(const Square& targetPos) const
{
    if (targetPos.x != square.x)
        return false;
    if (color == WHITE)
    {
        if (targetPos.y == square.y - 1)
            return true;
        if (targetPos.y == square.y - 2 && square.y == 6)
            return true;
        return false;
    }
    else
    {
        if (targetPos.y == square.y + 1)
            return true;
        if (targetPos.y == square.y + 2 && square.y == 1)
            return true;
        return false;
    }
}

bool Pawn::isAttacking(const Square& pos) const
{
    if (pos.x != square.x - 1 && pos.x != square.x + 1)
        return false;
    if (color == WHITE && pos.y == square.y - 1)
        return true;
    if (color == BLACK && pos.y == square.y + 1)
        return true;
    return false;
}

Square Pawn::getMovementVector(const Square& targetPos) const
{
    Square v = targetPos - square;
    if (color == WHITE && abs(v.x) == 1 && v.y == -1)
        return v;
    if (color == BLACK && abs(v.x) == 1 && v.y == 1)
        return v;
    return Piece::getMovementVector(targetPos);
}

// Knight methods
bool Knight::isReachable(const Square& targetPos) const
{
    Square vect = targetPos - square;
    return std::abs(vect.x * vect.y) == 2;
}

Square Knight::getMovementVector(const Square& targetPos) const
{
    if (isReachable(targetPos))
        return targetPos - square;
    return Square(8, 8);
}

// Bishop methods
bool Bishop::isReachable(const Square& targetPos) const
{
    Square vect = targetPos - square;
    return abs(vect.x) == abs(vect.y);
}

// Rook methods
bool Rook::isReachable(const Square& targetPos) const
{
    Square vect = targetPos - square;
    return vect.x == 0 || vect.y == 0;
}

// Queen methods
bool Queen::isReachable(const Square& targetPos) const
{
    Square vect = targetPos - square;
    return vect.x == 0 || vect.y == 0 || abs(vect.x) == abs(vect.y);
}

// King methods
bool King::isReachable(const Square& targetPos) const
{
    Square vect = targetPos - square;
    return abs(vect.x) <= 1 && abs(vect.y) <= 1;
}

Square King::getMovementVector(const Square& targetPos) const
{
    Square v = targetPos - square;
    if (color == WHITE && square.y == 7 && square.x == 4 && v.y == 0 && (v.x == 2 || v.x == -2))
        return v;
    if (color == BLACK && square.y == 0 && square.x == 4 && v.y == 0 && (v.x == 2 || v.x == -2))
        return v;
    return Piece::getMovementVector(targetPos);
}

