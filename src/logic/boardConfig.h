#pragma once

#include "bitboards.h"
#include "pieces.h"
#include "move.h"
#include <string>


// Extracts the common data used to quickly undo the last move
struct PositionInfo
{
	PositionInfo() : lastMove(NULL_MOVE) {}
	PositionInfo(const Move& move) : lastMove(move) {}
	PositionInfo& operator=(const PositionInfo& other);

	Move lastMove;

	int castlingRights = ALL_RIGHTS;
	Square enpassantSquare = INVALID_SQUARE;

	Bitboard checkers = 0;
	Bitboard pinned[COLOR_RANGE] = { 0 };
	Bitboard pinners[COLOR_RANGE] = { 0 };

	Piece capturedPiece = NO_PIECE;
	int halfmoveClock = 0;

	PositionInfo* prev = nullptr;
	PositionInfo* next = nullptr;
};



class BoardConfig
{
public:
	BoardConfig();
	~BoardConfig();

	// Static position setup
	void loadFromFen(const std::string& FEN);		// General static loading method
	void loadFromConfig(const BoardConfig& other);	// Used for a quick static loading of some other position with given BoardConfig

	// Move makers
	void makeMove(const Move& move);
	void normalMove(const Move& move);
	void promotion(const Move& move);
	void castle(const Move& move);
	void enpassant(const Move& move);
	void undoLastMove();

	// Piece-centric operations
	Bitboard pieces(PieceType ptype = ALL_PIECES) const;
	Bitboard pieces(Color side) const;
	template <typename... PieceTypes>
	Bitboard pieces(PieceType ptype, PieceTypes... types) const;
	template <typename... PieceTypes>
	Bitboard pieces(Color side, PieceTypes... types) const;
	template <typename... PieceTypes>
	Bitboard pieces(Color side, SquareColor squareColor, PieceTypes... types) const;
	Square kingPosition(Color side) const;

	// Square-centric operations
	Piece onSquare(Square sq) const;
	Bitboard attackersToSquare(Square sq, Color side, Bitboard occ) const;

	// Castling & enpassant
	Square enpassantSquare() const;
	bool hasCastlingRight(CastlingRights right) const;
	bool isCastlingPathClear(CastleType castle) const;

	// Checks & pins
	bool isInCheck(Color side) const;
	Bitboard checkingPieces() const;
	Bitboard pinnedPieces(Color side) const;	// Returns the pinned pieces of given color.
	Bitboard pinningPieces(Color side) const;	// Returns the pieces of opposite color pinning pieces of given color.

	// Move attributes checks
	bool legalityCheckLight(const Move& move) const;	// For interactions with move generator
	bool legalityCheckFull(const Move& move) const;		// For interactions with GUI & external move source

	// Move-counting issues & others
	Color movingSide() const;
	int gameStage() const;
	int halfmovesPlain() const;
	int halfmovesClocked() const;
	int moves() const;

	friend std::ostream& operator<<(std::ostream& os, const BoardConfig& board);

private:
	void clear();
	void pushStateList(const Move& lastMove);
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
	int gameStageValue = 0;

	PositionInfo* posInfo;
	PositionInfo* rootState;

	int halfmoveCount = 0;	// Counts the total number of moves for each side. To retrieve no. move: (halfmoveCount + 2 - sideOnMove) / 2
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

template <typename... PieceTypes>
inline Bitboard BoardConfig::pieces(Color side, SquareColor squareColor, PieceTypes... types) const
{
	return pieces(side, types...) & squaresOfColor(squareColor);
}

inline Square BoardConfig::kingPosition(Color side) const
{
	return kingSquare[side];
}

inline Piece BoardConfig::onSquare(Square sq) const
{
	return board[sq];
}

inline Square BoardConfig::enpassantSquare() const
{
	return posInfo->enpassantSquare;
}

inline bool BoardConfig::hasCastlingRight(CastlingRights right) const
{
	return (posInfo->castlingRights & right) == right;
}

inline bool BoardConfig::isCastlingPathClear(CastleType castle) const
{
	return !(pieces() & Pieces::castlingPath(castle));
}

inline bool BoardConfig::isInCheck(Color side) const
{
	return side == sideOnMove && posInfo->checkers != 0;
}

inline Bitboard BoardConfig::checkingPieces() const
{
	return posInfo->checkers;
}

inline Bitboard BoardConfig::pinnedPieces(Color side) const
{
	return posInfo->pinned[side];
}

inline Bitboard BoardConfig::pinningPieces(Color side) const
{
	return posInfo->pinners[side];
}

inline Color BoardConfig::movingSide() const
{
	return sideOnMove;
}

inline int BoardConfig::gameStage() const
{
	return gameStageValue > GAME_STAGE_MAX_VALUE ? (GAME_STAGE_MAX_VALUE >> 3) :
												   (gameStageValue >> 3);
}

inline int BoardConfig::halfmovesPlain() const
{
	return halfmoveCount;
}

inline int BoardConfig::halfmovesClocked() const
{
	return posInfo->halfmoveClock;
}

inline int BoardConfig::moves() const
{
	return (halfmoveCount + 2 - sideOnMove) >> 1;
}

inline void BoardConfig::pushStateList(const Move& lastMove)
{
	if (posInfo->next == nullptr) {
		posInfo->next = new PositionInfo(lastMove);
		posInfo->next->prev = posInfo;
	}
	posInfo = posInfo->next;
	posInfo->lastMove = lastMove;
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
	board[square] = NO_PIECE;
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
	posInfo->checkers = attackersToSquare(kingSquare[checkedSide], ~checkedSide, pieces());	// TO DO: It's not needed to check king attacks in order to detect checks
}