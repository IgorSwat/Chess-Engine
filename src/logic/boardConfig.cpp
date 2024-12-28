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
		posInfo->irrMovePlyDist = 0;
		posInfo->castlingRights &= castlingRightsLoss[to];
		posInfo->gameStageValue = posInfo->prev->gameStageValue - Evaluation::PieceStageInfluence[posInfo->capturedPiece];
		zobrist.updateByPlacementChange(board[to], to);
		removePiece(to);
	}
	else {
		posInfo->halfmoveClock = type_of(board[from]) == PAWN ? 0 : posInfo->prev->halfmoveClock + 1;
		posInfo->irrMovePlyDist = type_of(board[from]) == PAWN ? 0 : posInfo->prev->irrMovePlyDist + 1;
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
	posInfo->irrMovePlyDist = 0;
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
	posInfo->irrMovePlyDist = 0;
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
	posInfo->irrMovePlyDist = 0;
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

void BoardConfig::makeNullMove()
{
	pushStateList(Move::null());

	// Dynamic update
	sideOnMove = ~sideOnMove;
	zobrist.updateBySideOnMoveChange();
	updateChecks(sideOnMove);

	// Fill in other state fields with previous values
	posInfo->castlingRights = posInfo->prev->castlingRights;
	std::copy(posInfo->prev->discoveries, posInfo->prev->discoveries + COLOR_RANGE, posInfo->discoveries);
	std::copy(posInfo->prev->pinned, posInfo->prev->pinned + COLOR_RANGE, posInfo->pinned);
	std::copy(posInfo->prev->pinners, posInfo->prev->pinners + COLOR_RANGE, posInfo->pinners);
	posInfo->halfmoveClock = posInfo->prev->halfmoveClock;
	posInfo->gameStageValue = posInfo->prev->gameStageValue;
	posInfo->hash = zobrist.getHash();
	posInfo->irrMovePlyDist = posInfo->prev->irrMovePlyDist;	// TODO: Is this correct?
}

void BoardConfig::undoNullMove()
{
	sideOnMove = ~sideOnMove;
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

bool BoardConfig::isLegal(const Move& move) const
{
	Square from = move.from(), to = move.to();
	Piece piece = board[from];

	Color side = color_of(piece);
	Color enemy = ~side;

	// Special case - enpassant
	// It requires a check whether a move would create a discovered attack against our king
	if (move.isEnpassant()) {
		Bitboard occ = (pieces() ^ enpassantSquare() ^ from) | to;
		return !(Pieces::piece_attacks_s<BISHOP>(kingSquare[side], occ) & pieces(enemy, BISHOP, QUEEN)) &&
			   !(Pieces::piece_attacks_s<ROOK>(kingSquare[side], occ) & pieces(enemy, ROOK, QUEEN));
	}

	// Special case - castle
	if (move.isCastle()) {
		Direction dir = to > from ? EAST : WEST;

		// King can't castle via attacked squares
		return !attackersToSquare(from + dir, enemy, pieces()) &&
			   !attackersToSquare(to, enemy, pieces());
	}

	if (type_of(piece) == KING)
		return !attackersToSquare(to, enemy, pieces() ^ from);

	// Finally, check for pins
	return !(pinnedPieces(side) & from) || Board::aligned(kingSquare[side], to, from);
}

bool BoardConfig::isPseudolegal(const Move& move) const
{
	Square from = move.from(), to = move.to();
	Piece piece = board[from];

	// There has to be a piece on starting square of the move
	if (piece == NO_PIECE)
		return false;
	
	Color side = color_of(piece);
	Color enemy = ~side;

	if (side != movingSide())
		return false;

	// If target square is not empty, it cannot be occupied by friendly piece and move has to be capture
	if (board[to] != NO_PIECE && (!move.isCapture() || color_of(board[to]) == side))
		return false;

	// If target square is empty, then move cannot be a capture
	if (board[to] == NO_PIECE && move.isCapture() && !move.isEnpassant())
		return false;

	// Ensure that move is a check evasion if the side is in check
	if (isInCheck(side)) {
		// Piece moves
		if (type_of(piece) != KING) {
			// A piece move can never be a check evasion in case of double check
			if (!Bitboards::single_populated(posInfo->checkers))
				return false;

			Square checkSquare = Bitboards::lsb(posInfo->checkers);
			if (to != checkSquare && !Board::aligned(kingSquare[side], to, checkSquare))
				return false;
		}
		// King moves
		else if (attackersToSquare(to, enemy, pieces() ^ from))
			return false;
	}

	// Special case - castle
	if (move.isCastle()) {
		CastleType castleType = make_castle_type(side, to > from ? KINGSIDE_CASTLE : QUEENSIDE_CASTLE);
		
		if (!hasCastlingRight(castleType) || !isCastlingPathClear(castleType) || isInCheck(side))
			return false;
	}
	// Special case - pawn moves
	else if (type_of(piece) == PAWN) {
		Direction forward = side == WHITE ? NORTH : SOUTH;
		Bitboard secondRank = side == WHITE ? Board::RANK_2 : Board::RANK_7;

		if (!(move.isEnpassant() && enpassantSquare() != INVALID_SQUARE &&
				Board::AdjacentRankSquares[enpassantSquare()] & from && to == enpassantSquare() + forward) &&			// Enpassant														// Enpassant
			!(move.isCapture() && !move.isEnpassant() && Pieces::pawn_attacks(side, from) & to) &&												// Capture
			!(to == from + forward && board[to] == NO_PIECE) &&															// 1-push
			!(to == from + forward + forward && !(Board::Paths[from][to] & (pieces() ^ from)) && secondRank & from))	// 2-push
			return false;
	}
	// Common moves - moving correctness
	else if (!(Pieces::piece_attacks_d(type_of(piece), from, pieces()) & to))
		return false;
	
	return true;
}


// --------------------
// Move counting issues
// --------------------

std::uint16_t BoardConfig::countRepetitions() const
{
	PositionInfo* curr = posInfo;
	std::uint64_t targetHash = curr->hash;
	std::uint16_t loops = curr->irrMovePlyDist >> 1;	// Equivalent to "/ 2"
	std::uint16_t count = 1;

	for (std::uint16_t i = 0; i < loops; i++) {
		curr = curr->prev->prev;
		if (curr->hash == targetHash)
			count++;
	}

	return count;
}


// --------------------
// Board state handlers
// --------------------

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