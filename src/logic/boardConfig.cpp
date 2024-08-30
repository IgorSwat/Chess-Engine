#pragma once

#include "boardConfig.h"
#include "../engine/evaluationConfig.h"
#include <iostream>
#include <cctype>
#include <sstream>
#include <bitset>
#include <algorithm>


namespace {

	const std::string STARTING_POS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	// Defines the possibly remaining castle rights after a piece moves from given square
	// It effectively works only during first moves of king and rooks
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

}


// ------------------
// Position properties
// -------------------

PositionInfo& PositionInfo::operator=(const PositionInfo& other)
{
	lastMove = other.lastMove;
	castlingRights = other.castlingRights;
	enpassantSquare = other.enpassantSquare;
	checkers = other.checkers;
	std::copy(other.checkArea, other.checkArea + PIECE_TYPE_RANGE, this->checkArea);
	std::copy(other.discoveries, other.discoveries + COLOR_RANGE, this->discoveries);
	std::copy(other.pinned, other.pinned + COLOR_RANGE, this->pinned);
	std::copy(other.pinners, other.pinners + COLOR_RANGE, this->pinners);
	capturedPiece = other.capturedPiece;
	halfmoveClock = other.halfmoveClock;
	hash = other.hash;

	return *this;
}


// ----------------------------------------
// Initialization & static position loading
// ----------------------------------------


BoardConfig::BoardConfig()
{
	rootState = new PositionInfo();
	posInfo = rootState;
	loadPosition();
}

BoardConfig::~BoardConfig()
{
	while (posInfo != nullptr) {
		PositionInfo* tmp = posInfo;
		posInfo = posInfo->prev;
		delete tmp;
	}
}

void BoardConfig::loadPosition()
{
	loadPosition(STARTING_POS);
}

void BoardConfig::loadPosition(const std::string& fen)
{
	clear();

	std::string part;
	std::istringstream stream(fen);

	// Part 1 - piece placement
	stream >> part;
	int rank = 7, file = 0;		// In FEN we start from upper side of a board
	for (char symbol : part) {
		if (symbol == '/') {
			rank--;		// Go to another rank
			file = 0;   // Reset file id
		}
		else if (std::isdigit(symbol))
			file += static_cast<int>(symbol - '0');
		else {
			Square square = make_square(rank, file);
			Color color = std::isupper(symbol) ? WHITE : BLACK;
			PieceType type = std::tolower(symbol) == 'p' ? PAWN :
							 std::tolower(symbol) == 'n' ? KNIGHT :
							 std::tolower(symbol) == 'b' ? BISHOP :
							 std::tolower(symbol) == 'r' ? ROOK :
							 std::tolower(symbol) == 'q' ? QUEEN : KING;
			placePiece(make_piece(color, type), square);
			posInfo->gameStageValue += Evaluation::PieceStageInfluence[make_piece(color, type)];
			file++;
		}
	}

	// Part 2 - side on move
	stream >> part;
	sideOnMove = part == "w" ? WHITE : BLACK;

	// Part 3 - castling rights
	stream >> part;
	if (part.find('K') != std::string::npos)
		posInfo->castlingRights |= WHITE_OO;
	if (part.find('Q') != std::string::npos)
		posInfo->castlingRights |= WHITE_OOO;
	if (part.find('k') != std::string::npos)
		posInfo->castlingRights |= BLACK_OO;
	if (part.find('q') != std::string::npos)
		posInfo->castlingRights |= BLACK_OOO;
	
	// Part 4 - enpassant
	stream >> part;
	if (part != "-") {
		int epRank = sideOnMove == WHITE ? 4 : 3;
		int epFile = static_cast<int>(part.front() - 'a');
		posInfo->enpassantSquare = make_square(epRank, epFile);
	}

	// Part 5 - move counters
	stream >> posInfo->halfmoveClock;
	stream >> halfmoveCount;
	halfmoveCount = halfmoveCount * 2 + sideOnMove - 2;

	// State updates
	updateChecks(sideOnMove);
	updatePins(WHITE);
	updatePins(BLACK);

	zobrist.generateHash(this);
	posInfo->hash = zobrist.getHash();
}

void BoardConfig::loadPosition(const BoardConfig& other)
{
	posInfo = rootState;
	*posInfo = *other.posInfo;

	sideOnMove = other.sideOnMove;
	std::copy(other.board, other.board + SQUARE_RANGE, this->board);
	std::copy(other.piecesByType, other.piecesByType + PIECE_TYPE_RANGE, this->piecesByType);
	std::copy(other.piecesByColor, other.piecesByColor + COLOR_RANGE, this->piecesByColor);
	std::copy(other.kingSquare, other.kingSquare + COLOR_RANGE, this->kingSquare);
	posInfo->gameStageValue = other.posInfo->gameStageValue;
	halfmoveCount = other.halfmoveCount;

	zobrist.restoreHash(posInfo->hash);
}


// -----------
// Move-makers
// -----------

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
	Square rookFrom = getRookFromCastlingSquare(kingTo);
	Square rookTo = getRookToCastlingSquare(kingTo);

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
		movePiece(getRookToCastlingSquare(to), getRookFromCastlingSquare(to));	// Rook
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


// -------------------------
// Square-centric operations
// -------------------------

// Attacks from both sides
Bitboard BoardConfig::attackersToSquare(Square sq, Bitboard occ) const
{
	return Pieces::pawn_attacks(WHITE, sq) & pieces(BLACK, PAWN) | 
		   Pieces::pawn_attacks(BLACK, sq) & pieces(WHITE, PAWN) |
		   Pieces::piece_attacks_s<KNIGHT>(sq) & pieces(KNIGHT) |
		   Pieces::piece_attacks_s<BISHOP>(sq, occ) & pieces(BISHOP, QUEEN) |
		   Pieces::piece_attacks_s<ROOK>(sq, occ) & pieces(ROOK, QUEEN) |
		   Pieces::piece_attacks_s<KING>(sq) & pieces(KING);
}

// Attacks from given side
Bitboard BoardConfig::attackersToSquare(Square sq, Color side, Bitboard occ) const
{
	Bitboard attackdef = (Pieces::pawn_attacks(~side, sq) & pieces(PAWN)) |
		(Pieces::piece_attacks_s<KNIGHT>(sq) & pieces(KNIGHT)) |
		(Pieces::piece_attacks_s<BISHOP>(sq, occ) & pieces(BISHOP, QUEEN)) |
		(Pieces::piece_attacks_s<ROOK>(sq, occ) & pieces(ROOK, QUEEN)) |
		(Pieces::piece_attacks_s<KING>(sq) & pieces(KING));
	return attackdef & pieces(side);
}


// ---------------
// Legality checks
// ---------------

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

	// Target square cannot be already occupied by friendly piece
	if (pieces(side) & to) 
		return false;

	// Special case 1 - enpassant
	if (move.isEnpassant()) {
		if (!(Board::AdjacentRankSquares[enpassantSquare()] & from) || file_of(enpassantSquare()) != file_of(to)) 
			return false;
		Bitboard occ = (pieces() ^ enpassantSquare() ^ from) | to;
		return !attackersToSquare(kingSquare[side], ~side, occ) ||
			(checkingPieces() && ((Bitboards::single_populated(checkingPieces())) && enpassantSquare() == Bitboards::lsb(checkingPieces())));
	}

	// Special case 2 - castle
	if (move.isCastle()) {
		CastleType castleType = make_castle_type(side, to > from ? KINGSIDE_CASTLE : QUEENSIDE_CASTLE);
		if (!hasCastlingRight(castleType)) return false;
		if (!isCastlingPathClear(castleType)) return false;
		Direction dir = (castleType & KINGSIDE_CASTLE) ? EAST : WEST;
		return attackersToSquare(from + dir, ~side, pieces()) == 0 &&
			attackersToSquare(to, ~side, pieces()) == 0 && !isInCheck(side);
	}

	// Eliminate moves that violetes piece moving rules
	if ((type_of(piece) == PAWN && !Pieces::in_pawn_range(side, from, to) && !(Pieces::pawn_attacks(side, from) & to)) ||
		(type_of(piece) != PAWN && !Pieces::in_piece_range(type_of(piece), from, to)))
		return false;

	if (type_of(piece) == KING)
		return !attackersToSquare(to, ~side, pieces() ^ from);	// The "to" square cannot be in check after king move

	// Handle any other moves
	Square kingSq = kingSquare[side];
	if (checkingPieces() && !Bitboards::single_populated(checkingPieces())) return false;			// Double check blocks any other pieces than king
	if (checkingPieces() && to != Bitboards::lsb(checkingPieces()) && !Board::aligned(kingSq, Bitboards::lsb(checkingPieces()), to)) return false;
	Bitboard between = Board::Paths[from][to] & (~from) & (~to);
	return !(between & pieces()) && (!(pinnedPieces(side) & from) || Board::aligned(kingSq, to, from));
}


// --------------------
// Board state handlers
// ---------------------

void BoardConfig::clear()
{
	posInfo = rootState;

	std::fill(piecesByType, piecesByType + PIECE_TYPE_RANGE, 0);
	std::fill(piecesByColor, piecesByColor + COLOR_RANGE, 0);
	std::fill(board, board + SQUARE_RANGE, NO_PIECE);
	kingSquare[WHITE] = kingSquare[BLACK] = INVALID_SQUARE;

	posInfo->castlingRights = NO_CASTLING;
	posInfo->enpassantSquare = INVALID_SQUARE;
	posInfo->checkers = 0;

	halfmoveCount = 0;
	posInfo->halfmoveClock = 0;
	posInfo->gameStageValue = 0;
}

void BoardConfig::updatePins(Color side)
{
	Color enemy = ~side;
	posInfo->pinned[side] = posInfo->pinners[enemy] = posInfo->discoveries[enemy] = 0;

	// Consider all xray attackers, regardless of whether they are attacking through our piece (pin) or their piece (potential discovery)
	Bitboard xrayAttackers = (Pieces::xray_attacks<ROOK>(kingSquare[side], pieces(), pieces()) & pieces(enemy, ROOK, QUEEN)) |
							 (Pieces::xray_attacks<BISHOP>(kingSquare[side], pieces(), pieces()) & pieces(enemy, BISHOP, QUEEN));
	while (xrayAttackers) {
		Square sq = Bitboards::pop_lsb(xrayAttackers);
		Bitboard discovery = (Board::Paths[kingSquare[side]][sq] & pieces(enemy)) ^ sq;
		// Potential discovery
		if (discovery)
			posInfo->discoveries[enemy] |= discovery;
		// Pin
		else {
			posInfo->pinned[side] |= Board::Paths[kingSquare[side]][sq] & pieces(side);
			posInfo->pinners[enemy] |= sq;
		}
	}
	posInfo->pinned[side] &= ~kingSquare[side];
}


// -------------
// Miscellaneous
// -------------

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