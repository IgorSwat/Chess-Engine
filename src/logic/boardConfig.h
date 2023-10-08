#pragma once

#include "bitboards.h"
#include "move.h"
#include <string>

class BoardConfig
{
public:
	BoardConfig();

	void loadFromFEN(const std::string& FEN);

	void makeMove(const Move& move);
	void normalMove(const Move& move);
	void promotion(const Move& move);
	void castle(const Move& move, CastlingRights castleType);
	void enpassant(const Move& move);

	friend std::ostream& operator<<(std::ostream& os, const BoardConfig& board);

private:
	void clear();
	void placePiece(Piece piece, Square square);
	void removePiece(Square square);
	void movePiece(Square from, Square to);

	Color sideOnMove = WHITE;

	Piece board[SQUARE_RANGE];
	Bitboard pieces[PIECE_RANGE];
	Bitboard piecesByColor[COLOR_RANGE];	// Grouping all of side's pieces

	int castlingRights;
	Bitboard enpassantFile;

	int halfmoveCount = 0;	// Counts the total number of moves for each side. To retrieve no. move: (halfmoveCount + 2 - sideOnMove) / 2
	int halfmoveClock = 0;
};



inline void BoardConfig::placePiece(Piece piece, Square square)
{
	Color side = colorOf(piece);
	Bitboard squareBB = squareToBB(square);
	board[square] = piece;
	pieces[piece] ^= squareBB;
	piecesByColor[side] ^= squareBB;
}

inline void BoardConfig::removePiece(Square square)
{
	Piece piece = board[square];
	Color side = colorOf(piece);
	Bitboard squareBB = squareToBB(square);
	board[square] = piece;
	pieces[piece] ^= squareBB;
	piecesByColor[side] ^= squareBB;
}

inline void BoardConfig::movePiece(Square from, Square to)
{
	Piece piece = board[from];
	Color side = colorOf(piece);
	Bitboard moveBB = squareToBB(from) | squareToBB(to);
	board[from] = NO_PIECE;
	board[to] = piece;
	// Don't forget about handling captures!
	pieces[piece] ^= moveBB;
	piecesByColor[side] ^= moveBB;
}