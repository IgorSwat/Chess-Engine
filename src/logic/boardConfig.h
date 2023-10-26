#pragma once

#include "bitboards.h"
#include "pieces.h"
#include "move.h"
#include <string>

class BoardConfig
{
public:
	BoardConfig();

	// Static position setup
	void loadFromFEN(const std::string& FEN);

	// Move makers
	void makeMove(const Move& move);
	void normalMove(const Move& move);
	void promotion(const Move& move);
	void castle(const Move& move);
	void enpassant(const Move& move);

	// Piece-centric operations
	Bitboard pieces(PieceType ptype = ALL_PIECES) const;
	Bitboard pieces(Color side) const;
	template <typename... PieceTypes>
	Bitboard pieces(PieceType ptype, PieceTypes... types) const;
	template <typename... PieceTypes>
	Bitboard pieces(Color side, PieceTypes... types) const;

	// Square-centric operations
	Bitboard attackersToSquare(Square sq, Color side, Bitboard occ) const;

	// Castling & enpassant

	// Checks & pins
	bool isInCheck(Color side) const;
	Bitboard checkingPieces() const;
	// Returns the pinned pieces of given color.
	Bitboard pinnedPieces(Color side) const;
	// Returns the pieces of opposite color pinning pieces of given color.
	Bitboard pinningPieces(Color side) const;

	friend std::ostream& operator<<(std::ostream& os, const BoardConfig& board);

private:
	void clear();
	// Piece coordination handlers
	void placePiece(Piece piece, Square square);
	void removePiece(Square square);
	void movePiece(Square from, Square to);
	// Checks & pins handlers
	void updateChecks(Color checkedSide);
	void updatePins(Color side);

	Color sideOnMove = WHITE;

	Piece board[SQUARE_RANGE];
	Bitboard piecesByType[PIECE_TYPE_RANGE];	// Grouping all pieces of given type
	Bitboard piecesByColor[COLOR_RANGE];		// Grouping all of side's pieces
	Square kingSquare[COLOR_RANGE];

	int castlingRights;
	Bitboard enpassantFile;

	Bitboard checkers;
	Bitboard pinned[COLOR_RANGE];
	Bitboard pinners[COLOR_RANGE];

	int halfmoveCount = 0;	// Counts the total number of moves for each side. To retrieve no. move: (halfmoveCount + 2 - sideOnMove) / 2
	int halfmoveClock = 0;
};



inline Bitboard BoardConfig::pieces(PieceType ptype) const
{
	return piecesByType[ptype];
}

inline Bitboard BoardConfig::pieces(Color side) const
{
	return piecesByColor[side];
}

template <typename... PieceTypes>
inline Bitboard BoardConfig::pieces(PieceType ptype, PieceTypes... types) const
{
	return pieces(ptype) | pieces(types...);
}

template <typename... PieceTypes>
inline Bitboard BoardConfig::pieces(Color side, PieceTypes... types) const
{
	return pieces(side) & pieces(types...);
}

inline Bitboard BoardConfig::attackersToSquare(Square sq, Color side, Bitboard occ) const
{
	return (Pieces::pawnAttacks(~side, sq) & pieces(side, PAWN)) |
		(Pieces::knightAttacks(sq) & pieces(side, KNIGHT)) |
		(Pieces::bishopAttacks(sq, occ) & pieces(side, BISHOP, QUEEN)) |
		(Pieces::rookAttacks(sq, occ) & pieces(side, ROOK, QUEEN)) |
		(Pieces::kingAttacks(sq) & pieces(side, KING));
}

inline bool BoardConfig::isInCheck(Color side) const
{
	return side == sideOnMove && checkers != 0;
}

inline Bitboard BoardConfig::checkingPieces() const
{
	return checkers;
}

inline Bitboard BoardConfig::pinnedPieces(Color side) const
{
	return pinned[side];
}

inline Bitboard BoardConfig::pinningPieces(Color side) const
{
	return pinners[side];
}

inline void BoardConfig::placePiece(Piece piece, Square square)
{
	Bitboard squareBB = squareToBB(square);
	Color side = colorOf(piece);
	PieceType type = typeOf(piece);
	board[square] = piece;
	piecesByColor[side] |= squareBB;
	piecesByType[type] |= squareBB;
	piecesByType[ALL_PIECES] |= squareBB;
	if (type == KING) kingSquare[side] = square;
}

inline void BoardConfig::removePiece(Square square)
{
	Piece piece = board[square];
	Bitboard squareBB = squareToBB(square);
	board[square] = piece;
	piecesByColor[colorOf(piece)] ^= squareBB;
	piecesByType[typeOf(piece)] ^= squareBB;
	piecesByType[ALL_PIECES] ^= squareBB;
	// We assume that king is never captured or removed from the board
}

inline void BoardConfig::movePiece(Square from, Square to)
{
	Piece piece = board[from];
	Color side = colorOf(piece);
	PieceType type = typeOf(piece);
	Bitboard moveBB = squareToBB(from) | squareToBB(to);
	board[from] = NO_PIECE;
	board[to] = piece;
	piecesByColor[side] ^= moveBB;
	piecesByType[type] ^= moveBB;
	piecesByType[ALL_PIECES] ^= moveBB;
	if (type == KING) kingSquare[side] = to;
}

inline void BoardConfig::updateChecks(Color checkedSide)
{
	checkers = attackersToSquare(kingSquare[checkedSide], ~checkedSide, pieces());
}

inline void BoardConfig::updatePins(Color side)
{
	Square kingSq = kingSquare[side];
	Color enemy = ~side;
	pinned[side] = 0;
	pinners[side] = (Pieces::xRayRookAttacks(kingSq, pieces(), pieces(side)) & pieces(enemy, ROOK, QUEEN)) |
		(Pieces::xRayBishopAttacks(kingSq, pieces(), pieces(side)) & pieces(enemy, BISHOP, QUEEN));
	Bitboard pinnersTmp = pinners[side];
	while (pinnersTmp) {
		Square sq = Bitboards::popLsb(pinnersTmp);
		pinned[side] |= (pathBetween(kingSq, sq) & pieces(side));
	}
	pinned[side] &= ~squareToBB(kingSq);	// Only if PATHS[sq1][sq2] contains sq1 and sq2
}