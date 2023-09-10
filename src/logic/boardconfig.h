#pragma once

#include "progressstack.h"
#include "fenparser.h"
#include "observable.h"
#include "../engine/zobrist.h"
#include "misc.h"
#include <vector>
#include <bitset>
#include <functional>

class MoveGenerator;
using SquaresVec = std::vector<Square>;

namespace BoardManipulation 
{
	extern const Square nullSquare;
	extern const Square nullSquareX9;
	extern const Direction& nullDirection;
	extern SquaresVec* nullCheckSaver;
	extern const Square kingPositionsAfterCastling[2][2];
	extern const Square rookPositionsAfterCastling[2][2];
	constexpr int enPassantRows[2]{ 3, 4 };

	bool isCorrectSquare(const Square& square);
	const Direction& toDirectionalVector(const Direction& unnormalizedVector);
	const Direction& toDirectionalVector(const Square& initPos, const Square& targetPos);
}

class BoardConfig : public MoveListChangedObserver
{
public:
	BoardConfig();
	BoardConfig(const BoardConfig& other);
	BoardConfig& operator=(const BoardConfig& other);
	virtual ~BoardConfig();

	bool setFromFEN(const std::string& FEN);
	void setToDefault();

	Side getSideOnMove() const { return sideOnMove; }
	const Piece* getPiece(int pieceID) const { return &pieces[pieceID]; }
	const Piece* getPiece(const Square& pos) const { return board[pos.y][pos.x]; }
	const Piece* getPiece(int x, int y) const { return board[y][x]; }
	const Piece* getKing(Side side) const { return &pieces[16 * int(side)]; }
	const SquaresVec& getPiecePlacement(Side side, PieceType type) const { return piecePositions[side][type]; }
	bool hasCastlingRight(Side side, CastleType castleType) const;
	const Direction& getPinDirection(const Piece* piece) const { return pinDirections[piece->getID()]; }
	const Direction& getPinDirection(int pieceID) const { return pinDirections[pieceID]; }
	bool isCoveringChecks(const Piece* piece, const Square& targetPos) const;
	const SquaresVec* getSquaresCoveringChecks(Side side) const { return safeMoves[side]; }
	bool isSquareChecked(const Square& square, Side checkingSide) const { return attacksTable[square.y][square.x][checkingSide] > 0; }
	bool isSquareChecked(int x, int y, Side checkingSide) const { return attacksTable[y][x][checkingSide]; }
	bool isInCheck(Side side) const { return checkers[opposition[side]][0] != nullptr; }
	const Piece* getKingUnderCheck() const;
	int isCastlingAvailable(Side side, CastleType castleType);	// Returns the ID of rook angaged in castling or -1 when castling is not available
	bool isEnPassantAvailable(const Piece* pawn) const;
	int getEnPassantLine() const { return enPassantLine; }
	int getMoveNumber() const { return moveNumber; }
	
	void handleNormalMove(const Move& move);
	void handleCastling(const Move& move);
	void handleEnPassant(const Move& move);
	void makeMove(const Move& move);
	bool undoLastMove();
	bool isMoveLegal(const Move& move);

	void addAttack(const Square& square, Side side) { attacksTable[square.y][square.x][side]++; }
	void removeAttack(const Square& square, Side side) { attacksTable[square.y][square.x][side]--; }
	void updateByInsertion(const Move& move) override;
	void updateByRemoval(int pieceID, const vector<Move>& moves, bool legal) override;
	void updateByRemoval(int pieceID, const MoveList& moves, bool legal) override;

	void addObserver(PositionChangedObserver* observer);
	void removeObserver(PositionChangedObserver* observer);
	void updateObserversByMove(int pieceID, const Square& oldPos, const Square& newPos);
	MoveGenerator* getConnectedMoveGenerator() { return moveGenerator; }
	void clearAttacksTable();

	void connectEngine(EngineObserver* engine);
	void disconnectEngine(EngineObserver* engine);
	void updateEngines();

	void showCustomStats() const;

	using SearchCondition = bool (*)(const Square&);
	using AttackingCondition = bool (*)(const Piece*);

	friend void PlacementChange::applyChange(BoardConfig* config) const;
	friend void CastlingChange::applyChange(BoardConfig* config) const;
	friend void EnPassantChange::applyChange(BoardConfig* config) const;
	friend void CheckerChange::applyChange(BoardConfig* config) const;
	friend void PromotionChange::applyChange(BoardConfig* config) const;
	friend void MoveCountChange::applyChange(BoardConfig* config) const;
	friend bool FenParsing::parseFenToConfig(const std::string& FEN, BoardConfig* config);
private:
	void clearProgressStack();
	void clearBoardMap();
	void clearBoard();
	void copyCommonData(const BoardConfig& config);
	void updatePiecePlacement(Side side, PieceType type, const Square& oldPos, const Square& newPos);
	void removePiece(Piece* piece, const Square& nullSquare = BoardManipulation::nullSquare);
	void placePiece(int pieceID, PieceType targetType, const Square& newPos, bool updateObserversFlag = true);
	void updateSideOnMove();
	void updateMoveCount(const Piece* movedPiece, bool captureFlag);
	void setCastlingRight(Side side, CastleType castleType, bool updatedRight);
	void updateCastlingRights(const Piece* movedPiece, const Square& oldPos);
	void registerEnPassantChange(int enPassanter);
	void setEnPassantLine(int line);
	void updateEnPassant(const Piece* movedPiece, const Square& oldPos, const Square& newPos);
	Piece* nextInDirection(const Square& start, const Direction& moveDir, SearchCondition stopCondition);
	void clearPins();
	void removePin(Side side, const Piece* foundPiece, int foundPieceID);
	void searchForPins(Side side, const Square& kingPos, const Square& dir);
	void updatePinsFromKing(Side side, const Square& square);
	void updatePinsFromKing(Side side);
	void updatePinsFromSquare(const Square& square);
	void updatePinsStatically();
	void updatePinsDynamically(const Piece* piece, const Square& oldPos, const Square& newPos);
	void clearChecks();
	void updateChecksInDirection(const Piece* king, const Direction& dir, AttackingCondition attackingCondition, int& checkCount);
	void updateChecksInDirection(const Piece* king, const Direction& dir, int& checkCount);
	void updateChecksInDirections(const Piece* king, const DirectionsVec& dirs,  AttackingCondition attackingCondition, int& checkCount);
	void updateChecksForKnight(Piece* knight, Side checkedSide, const Square& kingPos, int& checkCount);
	void updateChecksForPawn(const Square& square, const Square& kingPos, Side kingColor, int& checkCount);
	void updateChecksForSide(Side side);
	void updateChecksStatically();
	void updateChecksDynamically(Piece* piece, const Square& oldPos, const Square& newPos);

	std::vector<Piece> pieces;
	SquaresVec piecePositions[2][5] {};
	Piece* board[8][8] { nullptr };

	Side sideOnMove = WHITE;
	std::bitset<4> castlingRights;
	int enPassantLine = -2;
	int enPassanter1 = -1;
	int enPassanter2 = -1;
	unsigned int halfMoves = 0;
	unsigned int moveNumber = 1;

	Direction pinDirections[32] { Direction(8,8) };
	std::vector<int> pinnedPieces[2] {};

	short attacksTable[8][8][2] { 0 };
	Piece* checkers[2][2] { nullptr };
	SquaresVec* safeMoves[2] { nullptr };	// nullptr - all moves are safe, nullSavers - only legal king moves are safe, other - list of safe moves

	ProgressStack boardProgress;
	ZobristHash zobristHash;

	std::vector<PositionChangedObserver*> positionObservers;
	std::vector<EngineObserver*> connectedEngines;
	MoveGenerator* moveGenerator;
};