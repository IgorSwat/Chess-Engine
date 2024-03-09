#pragma once

#include "boardConfig.h"
#include "../engine/evaluationConfig.h"
#include <iostream>
#include <cctype>
#include <sstream>
#include <bitset>

namespace {

	const std::string STARTING_POS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	constexpr CastlingRights castlingRightsLoss[SQUARE_RANGE] = {
		NO_WHITE_OOO, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, BLACK_BOTH, ALL_RIGHTS, ALL_RIGHTS, NO_WHITE_OO,
		ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS,
		ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS,
		ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS,
		ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS,
		ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS,
		ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS,
		NO_BLACK_OOO, ALL_RIGHTS, ALL_RIGHTS, ALL_RIGHTS, WHITE_BOTH, ALL_RIGHTS, ALL_RIGHTS, NO_BLACK_OO,
	};

	// Requires providing a kingTo square
	constexpr Square rookCastlingFromSquares[SQUARE_RANGE] = {
		SQ_A1, SQ_A1, SQ_A1, SQ_A1, INVALID_SQUARE, SQ_H1, SQ_H1, SQ_H1,
		INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE,
		INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE,
		INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE,
		INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE,
		INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE,
		INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE,
		SQ_A8, SQ_A8, SQ_A8, SQ_A8, INVALID_SQUARE, SQ_H8, SQ_H8, SQ_H8,
	};

	constexpr Square rookCastlingToSquares[SQUARE_RANGE] = {
		SQ_D1, SQ_D1, SQ_D1, SQ_D1, INVALID_SQUARE, SQ_F1, SQ_F1, SQ_F1,
		INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE,
		INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE,
		INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE,
		INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE,
		INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE,
		INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE, INVALID_SQUARE,
		SQ_D8, SQ_D8, SQ_D8, SQ_D8, INVALID_SQUARE, SQ_F8, SQ_F8, SQ_F8,
	};

	constexpr int PieceExchangeValue[PIECE_TYPE_RANGE] = {0, 1, 3, 3, 5, 9, 1000, 0};


	// ----------------
	// Helper functions
	// ----------------

	Piece pieceFromChar(char c)
	{
		char lowerC = std::tolower(c);
		Piece piece = lowerC == 'p' ? W_PAWN :
			lowerC == 'n' ? W_KNIGHT :
			lowerC == 'b' ? W_BISHOP :
			lowerC == 'r' ? W_ROOK :
			lowerC == 'q' ? W_QUEEN :
			lowerC == 'k' ? W_KING : NO_PIECE;
		return (piece == NO_PIECE || std::isupper(c)) ? piece : Piece(piece | BLACK_PIECE);
	}

	CastlingRights castlingRightsFromChar(char c)
	{
		return c == 'K' ? WHITE_OO :
			c == 'Q' ? WHITE_OOO :
			c == 'k' ? BLACK_OO :
			c == 'q' ? BLACK_OOO : NO_CASTLING;
	}
}



PositionInfo& PositionInfo::operator=(const PositionInfo& other)
{
	lastMove = other.lastMove;
	castlingRights = other.castlingRights;
	enpassantSquare = other.enpassantSquare;
	checkers = other.checkers;
	pinned[WHITE] = other.pinned[WHITE];
	pinned[BLACK] = other.pinned[BLACK];
	pinners[WHITE] = other.pinners[WHITE];
	pinners[BLACK] = other.pinners[BLACK];
	capturedPiece = other.capturedPiece;
	halfmoveClock = other.halfmoveClock;
	hash = other.hash;
	return *this;
}



BoardConfig::BoardConfig()
{
	rootState = new PositionInfo();
	posInfo = rootState;
	loadFromFen(STARTING_POS);
}

BoardConfig::~BoardConfig()
{
	while (posInfo != nullptr) {
		PositionInfo* tmp = posInfo;
		posInfo = posInfo->prev;
		delete tmp;
	}
}

void BoardConfig::loadFromFen(const std::string& fen)
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
			placePiece(piece, square);
			posInfo->gameStageValue += Evaluation::PieceStageInfluence[piece];
			j++;
		}
	}
	sideOnMove = stream.get() == 'w' ? WHITE : BLACK;
	stream.get();
	while (!stream.eof() && (c = stream.get()) != ' ') {
		if (c != '-')
			posInfo->castlingRights |= castlingRightsFromChar(c);
	}
	if (!stream.eof() && (c = stream.get()) != '-') {
		int epRank = sideOnMove == WHITE ? 4 : 3;
		int epFile = static_cast<int>(c - 'a');
		posInfo->enpassantSquare = make_square(epRank, epFile);
	}
	stream >> posInfo->halfmoveClock;
	stream >> halfmoveCount;
	halfmoveCount = halfmoveCount * 2 + sideOnMove - 2;

	updateChecks(sideOnMove);
	updatePins(WHITE);
	updatePins(BLACK);

	zobrist.generateHash(this);
	posInfo->hash = zobrist.getHash();
}

void BoardConfig::loadFromConfig(const BoardConfig& other)
{
	posInfo = rootState;

	sideOnMove = other.sideOnMove;
	for (int i = 0; i < SQUARE_RANGE; i++) board[i] = other.board[i];
	for (int i = 0; i < PIECE_TYPE_RANGE; i++) piecesByType[i] = other.piecesByType[i];
	for (int i = 0; i < COLOR_RANGE; i++) { piecesByColor[i] = other.piecesByColor[i]; kingSquare[i] = other.kingSquare[i]; }
	posInfo->gameStageValue = other.posInfo->gameStageValue;
	halfmoveCount = other.halfmoveCount;

	*posInfo = *other.posInfo;

	zobrist.restoreHash(posInfo->hash);
}

void BoardConfig::makeMove(const Move& move)
{
	MoveType moveType = move.type();

	switch (moveType) {
	case NORMAL_MOVE:
		normalMove(move);
		break;
	case PROMOTION:
		promotion(move);
		break;
	case CASTLE:
		castle(move);
		break;
	case ENPASSANT:
		enpassant(move);
		break;
	}
}

void BoardConfig::normalMove(const Move& move)
{
	Square from = move.from();
	Square to = move.to();

	pushStateList(move);

	posInfo->castlingRights = posInfo->prev->castlingRights & castlingRightsLoss[from];
	zobrist.updateByCastlingRightsChange(posInfo->prev->castlingRights);
	zobrist.updateByCastlingRightsChange(posInfo->castlingRights);

	if (move.isCapture()) {
		posInfo->capturedPiece = board[to];
		posInfo->halfmoveClock = 0;
		posInfo->castlingRights &= castlingRightsLoss[to];
		posInfo->gameStageValue = posInfo->prev->gameStageValue - Evaluation::PieceStageInfluence[posInfo->capturedPiece];
		zobrist.updateByPlacementChange(board[to], to);
		removePiece(to);
	}
	else {
		posInfo->halfmoveClock = type_of(board[from]) == PAWN ? 0 : posInfo->prev->halfmoveClock + 1;
		posInfo->gameStageValue = posInfo->prev->gameStageValue;
	}
	zobrist.updateByPlacementChange(board[from], from);
	zobrist.updateByPlacementChange(board[from], to);
	movePiece(from, to);

	sideOnMove = ~sideOnMove;
	zobrist.updateBySideOnMoveChange();

	posInfo->enpassantSquare = move.isDoublePawnPush() ? to : INVALID_SQUARE;
	zobrist.updateByEnpassantChange(posInfo->prev->enpassantSquare);
	zobrist.updateByEnpassantChange(posInfo->enpassantSquare);

	halfmoveCount++;

	posInfo->hash = zobrist.getHash();

	// ---------- CHECKS & PINS ----------
	updateChecks(sideOnMove);
	updatePins(WHITE);
	updatePins(BLACK);

}

void BoardConfig::promotion(const Move& move)
{
	Square from = move.from();
	Square to = move.to();
	Color side = to > from ? WHITE : BLACK;
	PieceType promotionType = move.promotionType();
	Piece promotedPiece = make_piece(side, promotionType);

	pushStateList(move);
	posInfo->castlingRights = posInfo->prev->castlingRights;

	if (move.isCapture()) {
		posInfo->capturedPiece = board[to];
		posInfo->castlingRights &= castlingRightsLoss[to];
		zobrist.updateByCastlingRightsChange(posInfo->prev->castlingRights);
		zobrist.updateByCastlingRightsChange(posInfo->castlingRights);
		posInfo->gameStageValue = posInfo->prev->gameStageValue - Evaluation::PieceStageInfluence[posInfo->capturedPiece] 
																+ Evaluation::PieceStageInfluence[promotedPiece];
		zobrist.updateByPlacementChange(board[to], to);
		removePiece(to);
	}
	else
		posInfo->gameStageValue = posInfo->prev->gameStageValue + Evaluation::PieceStageInfluence[promotedPiece];
	zobrist.updateByPlacementChange(board[from], from);
	zobrist.updateByPlacementChange(promotedPiece, to);
	removePiece(from);
	placePiece(promotedPiece, to);

	sideOnMove = ~sideOnMove;
	zobrist.updateBySideOnMoveChange();

	posInfo->enpassantSquare = INVALID_SQUARE;
	zobrist.updateByEnpassantChange(posInfo->prev->enpassantSquare);
	zobrist.updateByEnpassantChange(INVALID_SQUARE);

	posInfo->halfmoveClock = 0;
	halfmoveCount++;

	posInfo->hash = zobrist.getHash();

	// ---------- CHECKS & PINS ----------
	updateChecks(sideOnMove);
	updatePins(WHITE);
	updatePins(BLACK);
}

void BoardConfig::castle(const Move& move)
{
	Square kingFrom = move.from();
	Square kingTo = move.to();
	Square rookFrom = rookCastlingFromSquares[kingTo];
	Square rookTo = rookCastlingToSquares[kingTo];

	pushStateList(move);

	posInfo->gameStageValue = posInfo->prev->gameStageValue;
	zobrist.updateByPlacementChange(board[kingFrom], kingFrom);
	zobrist.updateByPlacementChange(board[kingFrom], kingTo);
	zobrist.updateByPlacementChange(board[rookFrom], rookFrom);
	zobrist.updateByPlacementChange(board[rookFrom], rookTo);
	movePiece(kingFrom, kingTo);
	movePiece(rookFrom, rookTo);

	sideOnMove = ~sideOnMove;
	zobrist.updateBySideOnMoveChange();

	posInfo->castlingRights = posInfo->prev->castlingRights & castlingRightsLoss[kingFrom];
	zobrist.updateByCastlingRightsChange(posInfo->prev->castlingRights);
	zobrist.updateByCastlingRightsChange(posInfo->castlingRights);

	posInfo->enpassantSquare = INVALID_SQUARE;
	zobrist.updateByEnpassantChange(posInfo->prev->enpassantSquare);
	zobrist.updateByEnpassantChange(INVALID_SQUARE);

	posInfo->halfmoveClock = posInfo->prev->halfmoveClock + 1;
	halfmoveCount++;

	posInfo->hash = zobrist.getHash();

	// ---------- CHECKS & PINS ----------
	updateChecks(sideOnMove);
	updatePins(WHITE);	//
	updatePins(BLACK);	// TO DO: Is it possible to make it quicker when only rook can cause new pins to enemy?
}

void BoardConfig::enpassant(const Move& move)
{
	Square from = move.from();
	Square to = move.to();
	Square captureSquare = posInfo->enpassantSquare;

	pushStateList(move);

	posInfo->capturedPiece = board[captureSquare];
	posInfo->gameStageValue = posInfo->prev->gameStageValue;
	zobrist.updateByPlacementChange(posInfo->capturedPiece, captureSquare);
	zobrist.updateByPlacementChange(board[from], from);
	zobrist.updateByPlacementChange(board[from], to);
	removePiece(captureSquare);
	movePiece(from, to);

	sideOnMove = ~sideOnMove;
	zobrist.updateBySideOnMoveChange();

	posInfo->castlingRights = posInfo->prev->castlingRights;

	posInfo->enpassantSquare = INVALID_SQUARE;
	zobrist.updateByEnpassantChange(posInfo->prev->enpassantSquare);
	zobrist.updateByEnpassantChange(INVALID_SQUARE);

	posInfo->halfmoveClock = 0;
	halfmoveCount++;

	posInfo->hash = zobrist.getHash();

	// ---------- CHECKS & PINS ----------
	updateChecks(sideOnMove);
	updatePins(WHITE);
	updatePins(BLACK);
}

void BoardConfig::undoLastMove()
{
	if (posInfo->prev == nullptr)
		return;

	const Move& lastMove = posInfo->lastMove;
	Square from = lastMove.from();	// Now it becomes "to" square
	Square to = lastMove.to();	// Now it becomes "from" square
	MoveType moveType = lastMove.type();

	sideOnMove = ~sideOnMove;

	switch (moveType) {
	case NORMAL_MOVE:
		movePiece(to, from);
		if (lastMove.isCapture()) 
			placePiece(posInfo->capturedPiece, to);
		break;
	case PROMOTION:
		removePiece(to);
		if (lastMove.isCapture())
			placePiece(posInfo->capturedPiece, to);
		placePiece(make_piece(sideOnMove, PAWN), from);
		break;
	case CASTLE:
		movePiece(to, from);	// King
		movePiece(rookCastlingToSquares[to], rookCastlingFromSquares[to]);	// Rook
		break;
	case ENPASSANT:
		movePiece(to, from);
		placePiece(posInfo->capturedPiece, posInfo->prev->enpassantSquare);
		break;
	}

	halfmoveCount--;

	posInfo = posInfo->prev;

	zobrist.restoreHash(posInfo->hash);
}

void BoardConfig::clear()
{
	posInfo = rootState;

	for (int i = 0; i < PIECE_TYPE_RANGE; i++)
		piecesByType[i] = 0;
	for (int i = 0; i < COLOR_RANGE; i++)
		piecesByColor[i] = 0;
	for (int i = 0; i < SQUARE_RANGE; i++)
		board[i] = NO_PIECE;
	kingSquare[WHITE] = kingSquare[BLACK] = INVALID_SQUARE;

	posInfo->castlingRights = NO_CASTLING;
	posInfo->enpassantSquare = INVALID_SQUARE;

	posInfo->checkers = 0;

	halfmoveCount = 0;
	posInfo->halfmoveClock = 0;
	posInfo->gameStageValue = 0;
}

Bitboard BoardConfig::attackersToSquare(Square sq, Bitboard occ) const
{
	return ((Pieces::pawn_attacks(WHITE, sq) | Pieces::pawn_attacks(BLACK, sq)) & pieces(PAWN)) |
		(Pieces::piece_attacks_s<KNIGHT>(sq) & pieces(KNIGHT)) |
		(Pieces::piece_attacks_s<BISHOP>(sq, occ) & pieces(BISHOP, QUEEN)) |
		(Pieces::piece_attacks_s<ROOK>(sq, occ) & pieces(ROOK, QUEEN)) |
		(Pieces::piece_attacks_s<KING>(sq) & pieces(KING));
}

Bitboard BoardConfig::attackersToSquare(Square sq, Color side, Bitboard occ) const
{
	Bitboard result = (Pieces::pawn_attacks(~side, sq) & pieces(PAWN)) |
		(Pieces::piece_attacks_s<KNIGHT>(sq) & pieces(KNIGHT)) |
		(Pieces::piece_attacks_s<BISHOP>(sq, occ) & pieces(BISHOP, QUEEN)) |
		(Pieces::piece_attacks_s<ROOK>(sq, occ) & pieces(ROOK, QUEEN)) |
		(Pieces::piece_attacks_s<KING>(sq) & pieces(KING));
	return result & pieces(side);
}

bool BoardConfig::legalityCheckLight(const Move& move) const
{
	Square from = move.from();
	Square to = move.to();
	Piece piece = board[from];
	Color side = color_of(piece);
	Color enemy = ~side;

	if (move.isEnpassant()) {
		Bitboard occ = (pieces() ^ enpassantSquare() ^ from) | to;
		Square kingSq = kingSquare[side];
		return !(Pieces::piece_attacks_s<BISHOP>(kingSq, occ) & pieces(enemy, BISHOP, QUEEN)) &&
			!(Pieces::piece_attacks_s<ROOK>(kingSq, occ) & pieces(enemy, ROOK, QUEEN));
	}

	if (move.isCastle()) {
		Direction dir = to > from ? EAST : WEST;
		return !attackersToSquare(from + dir, enemy, pieces()) &&
			!attackersToSquare(to, enemy, pieces());
	}

	if (type_of(piece) == KING)
		return !attackersToSquare(to, enemy, pieces() ^ from);

	return !(pinnedPieces(side) & from) || Board::aligned(kingSquare[side], to, from);
}

bool BoardConfig::legalityCheckFull(const Move& move) const
{
	Square from = move.from();
	Square to = move.to();
	Piece piece = board[from];
	Color side = color_of(piece);
	if (pieces(side) & to) return false;	// Target square cannot be already occupied by friendly piece

	if (move.isEnpassant()) {
		if (!(Board::AdjacentRankSquares[enpassantSquare()] & from) || file_of(enpassantSquare()) != file_of(to)) return false;
		Square kingSq = kingSquare[side];
		Bitboard occ = (pieces() ^ enpassantSquare() ^ from) | to;
		return !attackersToSquare(kingSq, ~side, occ) ||
			(checkingPieces() && ((Bitboards::single_populated(checkingPieces())) && enpassantSquare() == Bitboards::lsb(checkingPieces())));
	}

	if (move.isCastle()) {
		CastleType castleType = make_castle_type(side, to > from ? KINGSIDE_CASTLE : QUEENSIDE_CASTLE);
		if (!hasCastlingRight(castleType)) return false;
		if (!isCastlingPathClear(castleType)) return false;
		Direction dir = (castleType & KINGSIDE_CASTLE) ? EAST : WEST;
		Color enemy = ~side;
		return attackersToSquare(from + dir, enemy, pieces()) == 0 &&
			attackersToSquare(to, enemy, pieces()) == 0 && !isInCheck(side);
	}

	if (type_of(piece) == KING)
		return !attackersToSquare(to, ~side, pieces() ^ from);	// The "to" square cannot be in check after king move

	// Handle any other moves
	Square kingSq = kingSquare[side];
	if (checkingPieces() && !Bitboards::single_populated(checkingPieces())) return false;			// Double check blocks any other pieces than king
	if (checkingPieces() && to != Bitboards::lsb(checkingPieces()) && !Board::aligned(kingSq, Bitboards::lsb(checkingPieces()), to)) return false;
	Bitboard between = Board::Paths[from][to] & (~from) & (~to);
	return !(between & pieces()) && (!(pinnedPieces(side) & from) || Board::aligned(kingSq, to, from));
}

int BoardConfig::see(Square from, PieceType attackingPiece, Square to, PieceType attackedPiece) const
{
	int gain[33], depth = sideOnMove;
	Bitboard mayXray = pieces() ^ pieces(KNIGHT) ^ pieces(KING);
	Bitboard occ = pieces();
	Bitboard fromSet = square_to_bb(from);
	Bitboard attackdef = attackersToSquare(to, occ);

	gain[depth] = PieceExchangeValue[attackedPiece];
	do {
		depth++;
		gain[depth] = PieceExchangeValue[attackingPiece] - gain[depth - 1];
		if (std::max(-gain[depth - 1], gain[depth]) < 0)
			break;
		attackdef ^= fromSet;
		if (fromSet & mayXray) {
			Bitboard xRayAttackers = (Pieces::xray_attacks<BISHOP>(to, occ, fromSet) & pieces(BISHOP, QUEEN)) |
									 (Pieces::xray_attacks<ROOK>(to, occ, fromSet) & pieces(ROOK, QUEEN));
			attackdef |= xRayAttackers;
		}
		occ ^= fromSet;
		fromSet = lvp(Color(depth & 0x1), attackdef, attackingPiece);
	} while (fromSet);
	while (--depth)
		gain[depth - 1] = -std::max(-gain[depth - 1], gain[depth]);
	return gain[sideOnMove];
}

void BoardConfig::updatePins(Color side)
{
	Square kingSq = kingSquare[side];
	Color enemy = ~side;
	posInfo->pinned[side] = 0;
	posInfo->pinners[side] = (Pieces::xray_attacks<ROOK>(kingSq, pieces(), pieces(side)) & pieces(enemy, ROOK, QUEEN)) |
		(Pieces::xray_attacks<BISHOP>(kingSq, pieces(), pieces(side)) & pieces(enemy, BISHOP, QUEEN));
	Bitboard pinnersTmp = posInfo->pinners[side];
	while (pinnersTmp) {
		Square sq = Bitboards::pop_lsb(pinnersTmp);
		posInfo->pinned[side] |= (Board::Paths[kingSq][sq] & pieces(side));
	}
	posInfo->pinned[side] &= ~kingSq;	// Only if PATHS[sq1][sq2] contains sq1 and sq2
}

Bitboard BoardConfig::lvp(Color side, Bitboard area, PieceType& piece) const
{
	for (piece = PAWN; piece <= KING; piece = PieceType(piece + 1)) {
		Bitboard subset = area & pieces(side, piece);
		if (subset)
			return square_to_bb(Bitboards::lsb(subset));
	}
	return 0;
}

std::ostream& operator<<(std::ostream& os, const BoardConfig& board)
{
	os << "---------- Side on move: " << (board.sideOnMove == WHITE ? "WHITE" : "BLACK") << std::endl;
	os << "---------- White pieces placement:\n" << Bitboards::to_string(board.piecesByColor[WHITE]) << std::endl;
	os << "---------- Black pieces placement:\n" << Bitboards::to_string(board.piecesByColor[BLACK]) << std::endl;
	os << "---------- Castling rights mask: " << std::bitset<4>(board.posInfo->castlingRights) << std::endl;
	os << "---------- En passant square:\n" << board.posInfo->enpassantSquare << std::endl;
	os << "---------- Move counts (half moves | half moves clock): " << board.halfmoveCount << " | " << board.posInfo->halfmoveClock << std::endl;
	return os;
}