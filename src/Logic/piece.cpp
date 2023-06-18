#include <cmath>
#include "piece.h"

using sf::Vector2i;
dirVector Piece::pawnDirs({Vector2i(0, 1)});
dirVector Piece::knightDirs({Vector2i(1, -2), Vector2i(2, -1), Vector2i(2, 1), Vector2i(1, 2), Vector2i(-1, 2),
                                Vector2i(-2, 1), Vector2i(-2, -1), Vector2i(-1, -2)});
dirVector Piece::bishopDirs({Vector2i(1, -1), Vector2i(1, 1), Vector2i(-1, 1), Vector2i(-1, -1)});
dirVector Piece::rookDirs({Vector2i(0, -1), Vector2i(1, 0), Vector2i(0, 1), Vector2i(-1, 0)});
dirVector Piece::queenDirs({Vector2i(0, -1), Vector2i(1, -1), Vector2i(1, 0), Vector2i(1, 1), Vector2i(0, 1),
                                Vector2i(-1, 1), Vector2i(-1, 0), Vector2i(-1, -1)});
dirVector King::initialSquares({Vector2i(4, 7), Vector2i(4, 0)});



COLOR operator++(COLOR color)
{
    if (color == COLOR::WHITE)
        return COLOR::BLACK;
    return COLOR::WHITE;
}



Vector2i Piece::getMovementVector(const sf::Vector2i& targetPos) const
{
    if (isReachable(targetPos))
    {
        Vector2i longVector = targetPos - square;
        int xOffset, yOffset;
        if (longVector.x == 0)
            xOffset = 0;
        else
            xOffset = longVector.x / abs(longVector.x);
        if (longVector.y == 0)
            yOffset = 0;
        else
            yOffset = longVector.y / abs(longVector.y);
        return Vector2i(xOffset, yOffset);
    }
    return Vector2i(8, 8);
}

// Pawn methods
bool Pawn::isReachable(const sf::Vector2i& targetPos) const
{
    if (targetPos.x != square.x)
        return false;
    if (color == COLOR::WHITE)
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

bool Pawn::isAttacking(const sf::Vector2i& pos) const
{
    if (pos.x != square.x - 1 && pos.x != square.x + 1)
        return false;
    if (color == COLOR::WHITE && pos.y == square.y - 1)
        return true;
    if (color == COLOR::BLACK && pos.y == square.y + 1)
        return true;
    return false;
}

Vector2i Pawn::getMovementVector(const sf::Vector2i& targetPos) const
{
    Vector2i v = targetPos - square;
    if (color == COLOR::WHITE && abs(v.x) == 1 && v.y == -1)
        return v;
    if (color == COLOR::BLACK && abs(v.x) == 1 && v.y == 1)
        return v;
    return Piece::getMovementVector(targetPos);
}

// Knight methods
bool Knight::isReachable(const sf::Vector2i& targetPos) const
{
    Vector2i vect = targetPos - square;
    return std::abs(vect.x * vect.y) == 2;
}

Vector2i Knight::getMovementVector(const sf::Vector2i& targetPos) const
{
    if (isReachable(targetPos))
        return targetPos - square;
    return Vector2i(8, 8);
}

// Bishop methods
bool Bishop::isReachable(const sf::Vector2i& targetPos) const
{
    Vector2i vect = targetPos - square;
    return abs(vect.x) == abs(vect.y);
}

// Rook methods
bool Rook::isReachable(const sf::Vector2i& targetPos) const
{
    Vector2i vect = targetPos - square;
    return vect.x == 0 || vect.y == 0;
}

// Queen methods
bool Queen::isReachable(const sf::Vector2i& targetPos) const
{
    Vector2i vect = targetPos - square;
    return vect.x == 0 || vect.y == 0 || abs(vect.x) == abs(vect.y);
}

// King methods
bool King::isReachable(const sf::Vector2i& targetPos) const
{
    Vector2i vect = targetPos - square;
    return abs(vect.x) <= 1 && abs(vect.y) <= 1;
}

Vector2i King::getMovementVector(const sf::Vector2i& targetPos) const
{
    Vector2i v = targetPos - square;
    if (color == COLOR::WHITE && square.y == 7 && square.x == 4 && v.y == 0 && (v.x == 2 || v.x == -2))
        return v;
    if (color == COLOR::BLACK && square.y == 0 && square.x == 4 && v.y == 0 && (v.x == 2 || v.x == -2))
        return v;
    return Piece::getMovementVector(targetPos);
}

