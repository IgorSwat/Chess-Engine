#pragma once

#include "misc.h"
#include <vector>

using Direction = Square;
using directionsVec = std::vector<Direction>;

class Piece2
{
public:
	Piece2(PieceType pieceType, Side side, int id, const Square& initialPosition);
	virtual ~Piece2() = default;

	void setPosition(const Square& pos) { position = pos; }
	int getID() const { return pieceID; }
	Side getColor() const { return color; }
	bool hasBishopAbilities() const { return bishopAbilities; }
	bool hasRookAbilities() const { return rookAbilities; }
private:
	const int pieceID;
	PieceType type;
	Side color;
	Square position;
	bool bishopAbilities = false;
	bool rookAbilities = false;
};