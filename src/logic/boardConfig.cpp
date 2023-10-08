#pragma once

#include "boardConfig.h"
#include <cctype>
#include <sstream>
#include <bitset>

namespace {
	const std::string STARTING_POS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	constexpr CastlingRights castlingRightsLoss[SQUARE_RANGE]{
		KINGSIDE_CASTLE, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, NO_CASTLING, ALL_RIGHTS, ALL_RIGHTS, QUEENSIDE_CASTLE,
		ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS,
		ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS,
		ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS,
		ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS,
		ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS,
		ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS,
		KINGSIDE_CASTLE, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, NO_CASTLING, ALL_RIGHTS, ALL_RIGHTS, QUEENSIDE_CASTLE,
	};

	using MoveMaker = void(*)(BoardConfig*, const Move&);
	const MoveMaker moveHandlers[MOVEMASK_SIZE]{
		[](BoardConfig* board, const Move& move) {board->normalMove(move); },
		[](BoardConfig* board, const Move& move) {board->normalMove(move); },
		[](BoardConfig* board, const Move& move) {board->castle(move, KINGSIDE_CASTLE); },
		[](BoardConfig* board, const Move& move) {board->castle(move, QUEENSIDE_CASTLE); },
		[](BoardConfig* board, const Move& move) {board->normalMove(move); },
		[](BoardConfig* board, const Move& move) {board->enpassant(move); },
		[](BoardConfig* board, const Move& move) { },
		[](BoardConfig* board, const Move& move) { },
		[](BoardConfig* board, const Move& move) {board->promotion(move); },
		[](BoardConfig* board, const Move& move) {board->promotion(move); },
		[](BoardConfig* board, const Move& move) {board->promotion(move); },
		[](BoardConfig* board, const Move& move) {board->promotion(move); },
		[](BoardConfig* board, const Move& move) {board->promotion(move); },
		[](BoardConfig* board, const Move& move) {board->promotion(move); },
		[](BoardConfig* board, const Move& move) {board->promotion(move); },
		[](BoardConfig* board, const Move& move) {board->promotion(move); },
	};

}



BoardConfig::BoardConfig()
{
	loadFromFEN(STARTING_POS);
}

void BoardConfig::loadFromFEN(const std::string& fen)
{
	clear();
	std::istringstream stream(fen);
	int i = 0, j = 0;
	char c;
	// Parsing the pieces distribution
	while (!stream.eof() && (c = stream.get()) != ' ') {
		if (c == '/') {
			i++;
			j = 0;
		}
		else if (std::isdigit(c))
			j += static_cast<int>(c - '0');
		else {
			Square square = Square((7 - i) * 8 + j);
			Piece piece = pieceFromChar(c);
			Color side = std::isupper(c) ? WHITE : BLACK;
			Bitboard bb = 1ULL << square;
			pieces[piece] |= bb;
			piecesByColor[side] |= bb;
			board[square] = piece;
			j++;
		}
	}
	sideOnMove = stream.get() == 'w' ? WHITE : BLACK;
	stream.get();
	while (!stream.eof() && (c = stream.get()) != ' ') {
		if (c != '-')
			castlingRights |= castlingRightsFromChar(c);
	}
	if (!stream.eof() && (c = stream.get()) != '-')
		enpassantFile = FILES[static_cast<int>(c - 'a')];
	stream >> halfmoveClock;
	stream >> halfmoveCount;
	halfmoveCount = halfmoveCount * 2 + sideOnMove - 2;
}

void BoardConfig::makeMove(const Move& move)
{
	moveHandlers[move.flags()](this, move);
}

void BoardConfig::normalMove(const Move& move)
{
	Square from = move.from();
	Square to = move.to();

	if (move.isCapture()) {
		removePiece(to);
		halfmoveClock = 0;
	}
	else 
		halfmoveClock = typeOf(board[from]) == PAWN ? 0 : halfmoveClock + 1;
	movePiece(from, to);

	sideOnMove = otherSide(sideOnMove);
	castlingRights &= castlingRightsLoss[from];
	enpassantFile = move.isDoublePawnPush() ? FILES[fileOf(to)] : 0;
	halfmoveCount++;
}

void BoardConfig::promotion(const Move& move)
{

}

void BoardConfig::castle(const Move& move, CastlingRights castleType)
{

}

void BoardConfig::enpassant(const Move& move)
{

}

void BoardConfig::clear()
{
	for (int i = 0; i < PIECE_RANGE; i++)
		pieces[i] = 0;
	for (int i = 0; i < COLOR_RANGE; i++)
		piecesByColor[i] = 0;
	for (int i = 0; i < SQUARE_RANGE; i++)
		board[i] = NO_PIECE;
	castlingRights = NO_CASTLING;
	enpassantFile = 0;
	halfmoveCount = 0;
	halfmoveClock = 0;
}

std::ostream& operator<<(std::ostream& os, const BoardConfig& board)
{
	os << "---------- Side on move: " << (board.sideOnMove == WHITE ? "WHITE" : "BLACK") << std::endl;
	os << "---------- White pieces placement:\n" << Bitboards::bitboardToString(board.piecesByColor[WHITE]) << std::endl;
	os << "---------- Black pieces placement:\n" << Bitboards::bitboardToString(board.piecesByColor[BLACK]) << std::endl;
	os << "---------- Castling rights mask: " << std::bitset<4>(board.castlingRights) << std::endl;
	os << "---------- En passant file bitboard:\n" << Bitboards::bitboardToString(board.enpassantFile) << std::endl;
	os << "---------- Move counts (half moves | half moves clock): " << board.halfmoveCount << " | " << board.halfmoveClock << std::endl;
	return os;
}
