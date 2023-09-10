#include "piece.h"

namespace PieceHandlers {
	DirectionsVec directions[6]{
		{Square(0, 1)},	// Pawn
		{Square(1, -2), Square(2, -1), Square(2, 1), Square(1, 2), Square(-1, 2), Square(-2, 1), Square(-2, -1), Square(-1, -2)},	// Knight
		{Square(1, -1), Square(1, 1), Square(-1, 1), Square(-1, -1)},	// Bishop
		{Square(0, -1), Square(1, 0), Square(0, 1), Square(-1, 0)},		//  Rook
		{Square(0, -1), Square(1, -1), Square(1, 0), Square(1, 1), Square(0, 1), Square(-1, 1), Square(-1, 0), Square(-1, -1)},		// Queen
		{Square(0, -1), Square(1, -1), Square(1, 0), Square(1, 1), Square(0, 1), Square(-1, 1), Square(-1, 0), Square(-1, -1)}		// King
	};

	const DirectionsVec& getMoveDirections(PieceType pieceType)
	{
		return directions[pieceType];
	}
}

Piece::Piece(PieceType pieceType, Side side, int id, const Square& initialPosition)
	: type(pieceType), color(side), pieceID(id), position(initialPosition)
{
	updateAbilities();
}

Piece::Piece(const Piece& other)
	: type(other.type), color(other.color), pieceID(other.pieceID), position(other.position), state(other.state)
{
	updateAbilities();
}

Piece& Piece::operator=(const Piece& other)
{
	type = other.type;
	color = other.color;
	position = other.position;
	state = other.state;
	updateAbilities();
	return *this;
}

void Piece::setType(PieceType newType)
{
	type = newType;
	updateAbilities();
}

void Piece::updateAbilities()
{
	bishopAbilities = type == BISHOP || type == QUEEN;
	rookAbilities = type == ROOK || type == QUEEN;
}