#pragma once

#include "misc.h"
#include <vector>

using Direction = Square;
using DirectionsVec = std::vector<Direction>;

namespace PieceHandlers {
	const DirectionsVec& getMoveDirections(PieceType pieceType);
}

class Piece
{
public:
	Piece(PieceType pieceType, Side side, int id, const Square& initialPosition);
	Piece(const Piece& other);
	Piece& operator=(const Piece& other);
	virtual ~Piece() = default;

	void setType(PieceType newType);
	void setPosition(const Square& pos) { position = pos; }
	void setState(bool newState) { state = newState; }
	int getID() const { return pieceID; }
	Side getColor() const { return color; }
	PieceType getType() const { return type; }
	const Square& getPosition() const { return position; }
	bool hasBishopAbilities() const { return bishopAbilities; }
	bool hasRookAbilities() const { return rookAbilities; }
	bool isActive() const { return state; }
	const DirectionsVec& getMoveDirections() const { return PieceHandlers::getMoveDirections(type); }
private:
	void updateAbilities();

	const int pieceID;
	PieceType type;
	Side color;
	Square position;
	bool bishopAbilities = false;
	bool rookAbilities = false;
	bool state = true;
};