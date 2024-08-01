#pragma once

#include "bitboards.h"
#include "pieces.h"
#include "move.h"
#include "zobrist.h"
#include "../engine/evaluationConfig.h"
#include <string>


// ------------------
// Position properties
// -------------------

// Extracts the common data used to quickly undo the last move
struct PositionInfo
{
	PositionInfo() : lastMove(Move::null()) {}
	PositionInfo(const Move& move) : lastMove(move) {}
	PositionInfo& operator=(const PositionInfo& other);

	Move lastMove;

	int castlingRights = ALL_RIGHTS;
	Square enpassantSquare = INVALID_SQUARE;

	Bitboard checkers = 0ULL;							// Bitboard map of current checkers (pieces that check the king of sideToMove)
	Bitboard checkArea[PIECE_TYPE_RANGE] = { 0ULL };	// Squares from which pieces of sideToMove can check enemy king
	Bitboard discoveries[COLOR_RANGE] = { 0ULL };		// Squares occupied by pieces which could cause a discovered check against enemy king if moved
	Bitboard pinned[COLOR_RANGE] = { 0ULL };			// Pinned pieces of given side
	Bitboard pinners[COLOR_RANGE] = { 0ULL };			// Pinners of given side (pieces that pins some enemy pieces)

	Piece capturedPiece = NO_PIECE;
	std::uint16_t halfmoveClock = 0;
	std::uint16_t gameStageValue = 0;

	std::uint64_t hash = 0ULL;

	PositionInfo* prev = nullptr;
	PositionInfo* next = nullptr;
};


// -------------------------------
// Main board representation class
// -------------------------------

class BoardConfig
{
public:
	BoardConfig();
	~BoardConfig();

	// Static position setup
	void loadPosition();							// Load starting position
	void loadPosition(const std::string& FEN);		// General static loading method
	void loadPosition(const BoardConfig& other);	// Used for a quick static loading of some other position with given BoardConfig

	// Move makers
	void makeMove(const Move& move);
	void undoLastMove();

	// Piece-centric operations
	Bitboard pieces(PieceType ptype = ALL_PIECES) const;
	Bitboard pieces(Color side) const;
	template <typename... PieceTypes> Bitboard pieces(PieceType ptype, PieceTypes... types) const;
	template <typename... PieceTypes> Bitboard pieces(Color side, PieceTypes... types) const;
	Square kingPosition(Color side) const;

	// Square-centric operations
	bool isFree(Square sq) const;
	Piece onSquare(Square sq) const;
	Bitboard attackersToSquare(Square sq, Bitboard occ) const;
	Bitboard attackersToSquare(Square sq, Color side, Bitboard occ) const;

	// Castling & enpassant
	Square enpassantSquare() const;
	int castlingRights() const;
	bool hasCastlingRight(CastlingRights right) const;
	bool isCastlingPathClear(CastleType castle) const;

	// Checks & pins
	bool isInCheck(Color side) const;
	Bitboard checkingPieces() const;
	Bitboard possibleChecks(PieceType ptype) const;	// Returns bitboard map of squares, from which piece of given type can deliver check to ~sideOnMove
	Bitboard pinnedPieces(Color side) const;	// Returns the pinned pieces of given color.
	Bitboard pinningPieces(Color side) const;	// Returns the pieces of opposite color pinning pieces of given color.

	// Move property issues
	bool legalityCheckLight(const Move& move) const;	// For interactions with move generator, should be used ONLY for moves generated with movegen
	bool legalityCheckFull(const Move& move) const;		// For interactions with GUI & external move source, should be used for external moves
	bool isCheck(const Move& move) const;

	// Move-counting issues & others
	Color movingSide() const;
	Move lastMove() const;
	std::uint16_t gameStage() const;
	std::uint16_t halfmovesPlain() const;
	std::uint16_t halfmovesClocked() const;
	std::uint16_t generatedMoves() const;
	std::uint64_t hash() const;

	friend std::ostream& operator<<(std::ostream& os, const BoardConfig& board);

private:
	// Global board state handlers
	void clear();
	void pushStateList(const Move& lastMove);
	void updateChecks(Color checkedSide);
	void updatePins(Color side);

	// Helper move-makers
	void normalMove(const Move& move);
	void promotion(const Move& move);
	void castle(const Move& move);
	void enpassant(const Move& move);

	// Piece coordination handlers
	void placePiece(Piece piece, Square square);
	void removePiece(Square square);
	void movePiece(Square from, Square to);

	// Other helper functions
	static Square getRookFromCastlingSquare(Square kingTo);
	static Square getRookToCastlingSquare(Square kingTo);

	Color sideOnMove = WHITE;
	// Piece-placement properties
	Piece board[SQUARE_RANGE];
	Bitboard piecesByType[PIECE_TYPE_RANGE];	// Grouping all pieces of given type
	Bitboard piecesByColor[COLOR_RANGE];		// Grouping all of side's pieces
	Square kingSquare[COLOR_RANGE];
	// Position info linked-list
	PositionInfo* posInfo;
	PositionInfo* rootState;
	// Move counting
	std::uint16_t halfmoveCount = 0;	// Counts the total number of moves for each side. To retrieve no. move: (halfmoveCount + 2 - sideOnMove) / 2
	
	// Zobrist subobject
	Zobrist::ZobristHash zobrist;
};


// ------------------------
// Piece-centric operations
// ------------------------

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

inline Square BoardConfig::kingPosition(Color side) const
{
	return kingSquare[side];
}


// -------------------------
// Square-centric operations
// -------------------------

inline bool BoardConfig::isFree(Square sq) const
{
	return board[sq] == NO_PIECE;
}

inline Piece BoardConfig::onSquare(Square sq) const
{
	return board[sq];
}


// --------------------
// Castling & enpassant
// --------------------

inline Square BoardConfig::enpassantSquare() const
{
	return posInfo->enpassantSquare;
}

inline int BoardConfig::castlingRights() const
{
	return posInfo->castlingRights;
}

inline bool BoardConfig::hasCastlingRight(CastlingRights right) const
{
	return (posInfo->castlingRights & right) == right;
}

inline bool BoardConfig::isCastlingPathClear(CastleType castle) const
{
	return !(pieces() & Pieces::castle_path(castle));
}


// -------------
// Checks & pins
// -------------

inline bool BoardConfig::isInCheck(Color side) const
{
	return side == sideOnMove && posInfo->checkers != 0;
}

inline Bitboard BoardConfig::checkingPieces() const
{
	return posInfo->checkers;
}

inline Bitboard BoardConfig::possibleChecks(PieceType ptype) const
{
	return posInfo->checkArea[ptype];
}

inline Bitboard BoardConfig::pinnedPieces(Color side) const
{
	return posInfo->pinned[side];
}

inline Bitboard BoardConfig::pinningPieces(Color side) const
{
	return posInfo->pinners[side];
}


// ---------------------
// Move attribute checks
// ---------------------

inline bool BoardConfig::isCheck(const Move& move) const
{
	Square from = move.from();
	Square to = move.to();
	return (posInfo->checkArea[type_of(board[from])] & to) ||
		   ((posInfo->discoveries[sideOnMove] & from) && !Board::aligned(kingSquare[~sideOnMove], to, from));
}


// -------------
// Other getters
// -------------

inline Color BoardConfig::movingSide() const
{
	return sideOnMove;
}

inline Move BoardConfig::lastMove() const
{
	return posInfo->lastMove;
}

inline std::uint16_t BoardConfig::gameStage() const
{
	return std::min(posInfo->gameStageValue, std::uint16_t(Evaluation::GAME_STAGE_MAX_VALUE));
}

inline std::uint16_t BoardConfig::halfmovesPlain() const
{
	return halfmoveCount;
}

inline std::uint16_t BoardConfig::halfmovesClocked() const
{
	return posInfo->halfmoveClock;
}

inline std::uint16_t BoardConfig::generatedMoves() const
{
	return (halfmoveCount + 2 - sideOnMove) >> 1;
}

inline std::uint64_t BoardConfig::hash() const
{
	return zobrist.getHash();
}


// --------------------
// Board state handlers
// --------------------

inline void BoardConfig::pushStateList(const Move& lastMove)
{
	if (posInfo->next == nullptr) {
		posInfo->next = new PositionInfo(lastMove);
		posInfo->next->prev = posInfo;
	}
	posInfo = posInfo->next;
	posInfo->lastMove = lastMove;
}

inline void BoardConfig::updateChecks(Color checkedSide)	// checkedSide = sideOnMove
{
	posInfo->checkers = attackersToSquare(kingSquare[checkedSide], ~checkedSide, pieces());	// TODO: It's not needed to consider king attacks in order to detect checks

	Square enemyKingPos = kingSquare[~checkedSide];
	posInfo->checkArea[PAWN] = Pieces::pawn_attacks(~checkedSide, enemyKingPos);
	posInfo->checkArea[KNIGHT] = Pieces::piece_attacks_s<KNIGHT>(enemyKingPos, pieces());
	posInfo->checkArea[BISHOP] = Pieces::piece_attacks_s<BISHOP>(enemyKingPos, pieces());
	posInfo->checkArea[ROOK] = Pieces::piece_attacks_s<ROOK>(enemyKingPos, pieces());
	posInfo->checkArea[QUEEN] = posInfo->checkArea[BISHOP] | posInfo->checkArea[ROOK];
}


// ------------------------
// Piece placement handlers
// ------------------------

inline void BoardConfig::placePiece(Piece piece, Square square)
{
	Bitboard squareBB = square_to_bb(square);
	Color side = color_of(piece);
	PieceType type = type_of(piece);
	board[square] = piece;
	piecesByColor[side] |= squareBB;
	piecesByType[type] |= squareBB;
	piecesByType[ALL_PIECES] |= squareBB;
	if (type == KING) kingSquare[side] = square;
}

inline void BoardConfig::removePiece(Square square)
{
	Piece piece = board[square];
	Bitboard squareBB = square_to_bb(square);
	board[square] = NO_PIECE;
	piecesByColor[color_of(piece)] ^= squareBB;
	piecesByType[type_of(piece)] ^= squareBB;
	piecesByType[ALL_PIECES] ^= squareBB;
	// We assume that king is never captured or removed from the board
}

inline void BoardConfig::movePiece(Square from, Square to)
{
	Piece piece = board[from];
	Color side = color_of(piece);
	PieceType type = type_of(piece);
	Bitboard moveBB = square_to_bb(from) | square_to_bb(to);
	board[from] = NO_PIECE;
	board[to] = piece;
	piecesByColor[side] ^= moveBB;
	piecesByType[type] ^= moveBB;
	piecesByType[ALL_PIECES] ^= moveBB;
	if (type == KING) kingSquare[side] = to;
}


// ----------------
// Helper functions
// ----------------

// Returns rook's starting square during castling (where castling type is indicated by king target square)
inline Square BoardConfig::getRookFromCastlingSquare(Square kingTo)
{
	return file_of(kingTo) == G_FILE ? kingTo + EAST : kingTo + WEST + WEST;
}

inline Square BoardConfig::getRookToCastlingSquare(Square kingTo)
{
	return file_of(kingTo) == G_FILE ? kingTo + WEST : kingTo + EAST;
}