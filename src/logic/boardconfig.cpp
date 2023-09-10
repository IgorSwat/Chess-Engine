#include "boardconfig.h"
#include "movegenerator.h"
#include <algorithm>
#include <functional>
#include <cmath>

namespace {
	const std::string DEFAULT_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

	constexpr int pawnFrontNum[2]{ -1, 1 };
	constexpr short castlingRightsMapping[2][2]{ {0, 1}, {2, 3} };
	const Square rookCastlingSquares[2][2]{ {Square(7, 7), Square(0, 7)}, {Square(7, 0), Square(0, 0)} };
	const Direction castlingVectors[2]{ Direction(2, 0), Direction(-2, 0) };
	const Direction castlingDirections[2]{ Direction(1, 0), Direction(-1, 0) };

	BoardConfig::AttackingCondition bishopAttackCondition = [](const Piece* piece) {return piece->hasBishopAbilities(); };
	BoardConfig::AttackingCondition rookAttackCondition = [](const Piece* piece) {return piece->hasRookAbilities(); };

	std::function<int(const Square&, const Square&)> pawnMoveDistance[2]{
		[](const Square& oldPos, const Square& newPos) {return oldPos.y - newPos.y; },
		[](const Square& oldPos, const Square& newPos) {return newPos.y - oldPos.y; }
	};

	bool isKnightAttackingSquare(const Square& knightPos, const Square& square)
	{
		int squaresDiff = (square.x - knightPos.x) * (square.y - knightPos.y);
		return squaresDiff == 2 || squaresDiff == -2;
	}

	bool isPawnAttackingSquare(Side pawnColor, const Square& pawnPos, const Square& square)
	{
		return square.y == pawnPos.y + pawnFrontNum[pawnColor] && (square.x == pawnPos.x - 1 || square.x == pawnPos.x + 1);
	}
}

namespace BoardManipulation {
	const Square nullSquare(8, 8);
	const Square nullSquareX9(9, 8);
	const Direction& nullDirection = nullSquare;

	const Square kingPositionsAfterCastling[2][2]{ {Square(6, 7), Square(2, 7)}, {Square(6, 0), Square(2, 0)} };
	const Square rookPositionsAfterCastling[2][2]{ {Square(5, 7), Square(3, 7)}, {Square(5, 0), Square(3, 0)} };

	Direction directionalVectors[15][15] {};
	bool directionalVectorsComputed = false;

	SquaresVec checkSavers[8][8][8][8] {};
	SquaresVec noCheckSavers {};
	SquaresVec* nullCheckSaver = &noCheckSavers;
	bool checkSaversComputed = false;

	Direction normaliseDirection(const Direction& mv)
	{
		if (mv.x == 0 && mv.y == 0)
			return Direction(8, 8);
		int x_uns = abs(mv.x);
		int y_uns = abs(mv.y);
		if (mv.x == 0)
			return Direction(mv.x, mv.y / y_uns);
		if (mv.y == 0)
			return Direction(mv.x / x_uns, mv.y);
		if (x_uns == y_uns)
			return Direction(mv.x / x_uns, mv.y / y_uns);
		return Direction(8, 8);
	}

	void initDirectionalVectors()
	{
		if (!directionalVectorsComputed)
		{
			for (int x = -7; x < 8; x++)
			{
				for (int y = -7; y < 8; y++)
					directionalVectors[7 - x][7 - y] = normaliseDirection(Direction(x, y));
			}
			directionalVectorsComputed = true;
		}
	}

	void initCheckSavers()
	{
		if (!checkSaversComputed)
		{
			for (int x1 = 0; x1 < 8; x1++)
			{
				for (int y1 = 0; y1 < 8; y1++)
				{
					for (int x2 = 0; x2 < 8; x2++)
					{
						for (int y2 = 0; y2 < 8; y2++)
						{
							Square p1(x1, y1);
							Square p2(x2, y2);
							if (p1 == p2)
								checkSavers[x1][y1][x2][y2].push_back(p1);
							else
							{
								Square dir{ toDirectionalVector(p1, p2) };
								if (dir.x < 8)
								{
									Square currPos{ p1 };
									while (currPos != p2)
									{
										checkSavers[x1][y1][x2][y2].push_back(currPos);
										currPos = currPos + dir;
									}
									checkSavers[x1][y1][x2][y2].push_back(p2);
								}
							}
						}
					}
				}
			}
			checkSaversComputed = true;
		}
	}

	bool isCorrectSquare(const Square& square)
	{
		return square.x >= 0 && square.x < 8 && square.y >= 0 && square.y < 8;
	}

	const Direction& toDirectionalVector(const Direction& unnormalizedVector)
	{
		return directionalVectors[7 - unnormalizedVector.x][7 - unnormalizedVector.y];
	}

	const Direction& toDirectionalVector(const Square& initPos, const Square& targetPos)
	{
		return toDirectionalVector(targetPos - initPos);
	}
}



// Global config changing
BoardConfig::BoardConfig()
{
	BoardManipulation::initDirectionalVectors();
	BoardManipulation::initCheckSavers();

	pieces.emplace_back(KING, WHITE, 0, Square(4, 7));
	for (int i = 0; i < 8; i++)
		pieces.emplace_back(PAWN, WHITE, i + 1, Square(i, 6));
	pieces.emplace_back(KNIGHT, WHITE, 9, Square(1, 7));
	pieces.emplace_back(KNIGHT, WHITE, 10, Square(6, 7));
	pieces.emplace_back(BISHOP, WHITE, 11, Square(2, 7));
	pieces.emplace_back(BISHOP, WHITE, 12, Square(5, 7));
	pieces.emplace_back(ROOK, WHITE, 13, Square(0, 7));
	pieces.emplace_back(ROOK, WHITE, 14, Square(7, 7));
	pieces.emplace_back(QUEEN, WHITE, 15, Square(3, 7));
	pieces.emplace_back(KING, BLACK, 16, Square(4, 7));
	for (int i = 0; i < 8; i++)
		pieces.emplace_back(PAWN, BLACK, i + 17, Square(i, 1));
	pieces.emplace_back(KNIGHT, BLACK, 25, Square(1, 0));
	pieces.emplace_back(KNIGHT, BLACK, 26, Square(6, 0));
	pieces.emplace_back(BISHOP, BLACK, 27, Square(2, 0));
	pieces.emplace_back(BISHOP, BLACK, 28, Square(5, 0));
	pieces.emplace_back(ROOK, BLACK, 29, Square(0, 0));
	pieces.emplace_back(ROOK, BLACK, 30, Square(7, 0));
	pieces.emplace_back(QUEEN, BLACK, 31, Square(3, 0));

	moveGenerator = new MoveGenerator(this);
	moveGenerator->addObserver(this);
	addObserver(moveGenerator);

	setToDefault();
	moveGenerator->generateAllMoves();
}

BoardConfig::BoardConfig(const BoardConfig& other)
{
	copyCommonData(other);
	pieces.reserve(32);
	for (int i = 0; i < 32; i++)
	{
		Piece* piece = &pieces.emplace_back(other.pieces[i]);
		if (piece->isActive())
		{
			board[piece->getPosition().y][piece->getPosition().x] = piece;
			updatePiecePlacement(piece->getColor(), piece->getType(), BoardManipulation::nullSquare, piece->getPosition());
		}
	}

	moveGenerator = new MoveGenerator(this);
	moveGenerator->addObserver(this);
	addObserver(moveGenerator);

	updatePinsStatically();
	updateChecksStatically();
	moveGenerator->generateAllMoves();
	zobristHash.generateHash(this);
}

BoardConfig& BoardConfig::operator=(const BoardConfig& other)
{
	clearBoardMap();
	clearProgressStack();
	copyCommonData(other);
	for (int i = 0; i < 32; i++)
	{
		PieceType oldType = pieces[i].getType();
		Square oldPos = pieces[i].isActive() ? pieces[i].getPosition() : BoardManipulation::nullSquare;
		pieces[i] = other.pieces[i];
		Piece* piece = &pieces[i];
		if (piece->isActive())
		{
			board[piece->getPosition().y][piece->getPosition().x] = piece;
			updatePiecePlacement(piece->getColor(), oldType, oldPos, BoardManipulation::nullSquare);
			updatePiecePlacement(piece->getColor(), piece->getType(), BoardManipulation::nullSquare, piece->getPosition());
		}
		else if (oldPos.x < 8)
			updatePiecePlacement(piece->getColor(), oldType, oldPos, BoardManipulation::nullSquare);
	}
	updatePinsStatically();
	updateChecksStatically();
	moveGenerator->generateAllMoves();
	zobristHash.generateHash(this);
	return *this;
}

BoardConfig::~BoardConfig()
{
	clearProgressStack();
	delete moveGenerator;
}

bool BoardConfig::setFromFEN(const std::string& FEN)
{
	clearBoard();
	if (FenParsing::parseFenToConfig(FEN, this))
	{
		updatePinsStatically();
		updateChecksStatically();
		updateEngines();
		moveGenerator->generateAllMoves();
		zobristHash.generateHash(this);
		return true;
	}
	return false;
}



// Position info
bool BoardConfig::hasCastlingRight(Side side, CastleType castleType) const
{
	return castlingRights[castlingRightsMapping[side][castleType]];
}

bool BoardConfig::isCoveringChecks(const Piece* piece, const Square& targetPos) const
{
	SquaresVec* safe = safeMoves[piece->getColor()];
	if (piece->getType() == KING || safe == nullptr)	// TO DO
		return true;
	if (safe == BoardManipulation::nullCheckSaver)
		return false;
	return std::find(safe->begin(), safe->end(), targetPos) != safe->end();
}

const Piece* BoardConfig::getKingUnderCheck() const
{
	if (isInCheck(WHITE))
		return getKing(WHITE);
	if (isInCheck(BLACK))
		return getKing(BLACK);
	return nullptr;
}

int BoardConfig::isCastlingAvailable(Side side, CastleType castleType)
{
	const Square& kingPos = getKing(side)->getPosition();
	const Direction& castlingVec = castlingVectors[castleType];
	if (!castlingRights[castlingRightsMapping[side][castleType]])
		return -1;
	if (isSquareChecked(kingPos.x + castlingVec.x / 2, kingPos.y, opposition[side]) || isSquareChecked(kingPos.x + castlingVec.x, kingPos.y, opposition[side]))
		return -1;
	const Piece* nextPiece = nextInDirection(kingPos, castlingDirections[castleType], BoardManipulation::isCorrectSquare);
	return nextPiece->getPosition() == rookCastlingSquares[side][castleType] ? nextPiece->getID() : -1;
}

bool BoardConfig::isEnPassantAvailable(const Piece* pawn) const
{
	const Square& pawnPos = pawn->getPosition();
	return pawnPos.y == BoardManipulation::enPassantRows[pawn->getColor()] && (pawnPos.x == enPassantLine - 1 || pawnPos.x == enPassantLine + 1);
}



// Move making
void BoardConfig::handleNormalMove(const Move& move)
{
	Piece* piece = &pieces[move.pieceID];
	Square oldPos(piece->getPosition());
	Piece* capturedPiece = board[move.targetPos.y][move.targetPos.x];
	bool captureFlag = false;
	if (capturedPiece != nullptr)
	{
		removePiece(capturedPiece, BoardManipulation::nullSquareX9);
		captureFlag = true;
	}
	board[oldPos.y][oldPos.x] = nullptr;
	boardProgress.registerChange(new PlacementChange(move.pieceID, oldPos));
	if (piece->getType() == PAWN && move.promotionType != PAWN)
	{
		placePiece(move.pieceID, move.promotionType, move.targetPos, false);
		boardProgress.registerChange(new PromotionChange(move.pieceID));
	}
	else
	{
		board[move.targetPos.y][move.targetPos.x] = piece;
		piece->setPosition(move.targetPos);
		updatePiecePlacement(piece->getColor(), piece->getType(), oldPos, move.targetPos);
		zobristHash.updateHash(piece->getColor(), piece->getType(), oldPos);
		zobristHash.updateHash(piece->getColor(), piece->getType(), move.targetPos);
	}

	updateChecksDynamically(piece, oldPos, move.targetPos);
	updatePinsDynamically(piece, oldPos, move.targetPos);
	updateEnPassant(piece, oldPos, move.targetPos);
	updateCastlingRights(piece, oldPos);
	updateSideOnMove();
	updateMoveCount(piece, captureFlag);
	boardProgress.pushChanges();
	updateObserversByMove(move.pieceID, oldPos, move.targetPos);
}

void BoardConfig::handleCastling(const Move& move)
{
	Piece* king = &pieces[move.pieceID];
	Square kingPos{ king->getPosition() };
	CastleType castleType = move.targetPos.x > king->getPosition().x ? SHORT_CASTLE : LONG_CASTLE;
	const Square& rookPos = rookCastlingSquares[king->getColor()][castleType];
	Piece* rook = board[rookPos.y][rookPos.x];
	const Square& rookTargetPos = BoardManipulation::rookPositionsAfterCastling[king->getColor()][castleType];
	placePiece(move.pieceID, KING, move.targetPos, false);
	placePiece(rook->getID(), ROOK, rookTargetPos, false);

	boardProgress.registerChange(new PlacementChange(king->getID(), Square(kingPos)));
	updateCastlingRights(king, kingPos);
	boardProgress.registerChange(new PlacementChange(rook->getID(), Square(rookPos)));
	updatePinsDynamically(king, kingPos, move.targetPos);
	updatePinsFromKing(opposition[king->getColor()], rookTargetPos);
	updateEnPassant(rook, rookPos, rookTargetPos);
	updateChecksDynamically(rook, rookPos, rookTargetPos);
	updateSideOnMove();
	updateMoveCount(king, false);
	boardProgress.pushChanges();
	updateObserversByMove(king->getID(), kingPos, move.targetPos);
	updateObserversByMove(rook->getID(), rookPos, rookTargetPos);
	
}

void BoardConfig::handleEnPassant(const Move& move)
{
	Piece* piece = &pieces[move.pieceID];
	Square oldPos{ piece->getPosition() };
	Piece* capturedPawn = board[BoardManipulation::enPassantRows[piece->getColor()]][move.targetPos.x];
	Square capturedPawnPos{ capturedPawn->getPosition() };
	removePiece(capturedPawn);
	updateObserversByMove(capturedPawn->getID(), capturedPawnPos, BoardManipulation::nullSquare);
	boardProgress.registerChange(new PlacementChange(move.pieceID, oldPos));
	placePiece(move.pieceID, PAWN, move.targetPos, false);

	updateChecksDynamically(piece, oldPos, move.targetPos);
	updatePinsDynamically(piece, oldPos, move.targetPos);
	updateEnPassant(piece, oldPos, move.targetPos);
	updateCastlingRights(piece, oldPos);
	updateSideOnMove();
	updateMoveCount(piece, true);
	boardProgress.pushChanges();
	updateObserversByMove(move.pieceID, oldPos, move.targetPos);
}

void BoardConfig::makeMove(const Move& move)
{
	if (move.hasProperty(Moves::CASTLE_FLAG))
		handleCastling(move);
	else if (move.hasProperty(Moves::ENPASSANT_FLAG))
		handleEnPassant(move);
	else
		handleNormalMove(move);
	updateEngines();
}

bool BoardConfig::undoLastMove()
{
	if (boardProgress.isEmpty())
		return false;
	updateSideOnMove();
	ChangesVec* changeLog = boardProgress.getLatestChanges();
	for (int i = changeLog->size() - 1; i >= 0; i--)
	{
		ConfigChange* change = changeLog->at(i);
		change->applyChange(this);
		delete change;
	}
	delete changeLog;
	updateEngines();
	return true;
}

bool BoardConfig::isMoveLegal(const Move& move)
{
	const MoveList& legalMoves = moveGenerator->getLegalMoves()[move.pieceID];
	auto moveIt = std::find(legalMoves.begin(), legalMoves.end(), move);
	return moveIt != legalMoves.end();
}



// Move list changed observer methods
void BoardConfig::updateByInsertion(const Move& move)
{
	if (move.hasProperty(Moves::ATTACK_FLAG))
		attacksTable[move.targetPos.y][move.targetPos.x][move.pieceID < 16 ? WHITE : BLACK] += 1;
}

void BoardConfig::updateByRemoval(int pieceID, const vector<Move>& moves, bool legal)
{
	Side side = pieceID < 16 ? WHITE : BLACK;
	for (const Move& move : moves)
	{
		if (move.hasProperty(Moves::ATTACK_FLAG))
			attacksTable[move.targetPos.y][move.targetPos.x][side] -= 1;
	}
}

void BoardConfig::updateByRemoval(int pieceID, const MoveList& moves, bool legal)
{
	Side side = pieceID < 16 ? WHITE : BLACK;
	for (const Move& move : moves)
	{
		if (move.hasProperty(Moves::ATTACK_FLAG))
			attacksTable[move.targetPos.y][move.targetPos.x][side] -= 1;
	}
}



// Handling the position observers
void BoardConfig::addObserver(PositionChangedObserver* observer)
{
	positionObservers.push_back(observer);
}

void BoardConfig::removeObserver(PositionChangedObserver* observer)
{
	auto it = std::find(positionObservers.begin(), positionObservers.end(), observer);
	if (it != positionObservers.end())
		positionObservers.erase(it);
}

void BoardConfig::updateObserversByMove(int pieceID, const Square& oldPos, const Square& newPos)
{
	for (PositionChangedObserver* observer : positionObservers)
		observer->updateByMove(pieceID, oldPos, newPos);
}



// Connecting & disconnecting engines
void BoardConfig::connectEngine(EngineObserver* engine)
{
	connectedEngines.push_back(engine);
}

void BoardConfig::disconnectEngine(EngineObserver* engine)
{
	auto it = std::find(connectedEngines.begin(), connectedEngines.end(), engine);
	if (it != connectedEngines.end())
		connectedEngines.erase(it);
}

void BoardConfig::updateEngines()
{
	for (EngineObserver* engine : connectedEngines)
		engine->reloadEngine();
}



void BoardConfig::showCustomStats() const
{
	std::cout << "Zobrist hash: " << zobristHash.getHash() << std::endl;
}



// Global config setters
void BoardConfig::clearProgressStack()
{
	while (!boardProgress.isEmpty())
	{
		ChangesVec* changes = boardProgress.getLatestChanges();
		for (ConfigChange* change : *changes)
			delete change;
		delete changes;
	}
}

void BoardConfig::clearAttacksTable()
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			attacksTable[i][j][WHITE] = 0;
			attacksTable[i][j][BLACK] = 0;
		}
	}
}

void BoardConfig::clearBoardMap()
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
			board[i][j] = nullptr;
	}
}

void BoardConfig::clearBoard()
{
	clearBoardMap();
	for (Piece& piece : pieces)
		piece.setState(false);
	for (std::vector<Square>& squares : piecePositions[WHITE])
		squares.clear();
	for (std::vector<Square>& squares : piecePositions[BLACK])
		squares.clear();
	for (int i = 0; i < 32; i++)
		pinDirections[i] = Direction(8, 8);
	pinnedPieces[WHITE].clear();
	pinnedPieces[BLACK].clear();
	castlingRights.reset();
	clearChecks();
	clearProgressStack();
}

void BoardConfig::setToDefault()
{
	setFromFEN(DEFAULT_FEN);
}

void BoardConfig::copyCommonData(const BoardConfig& config)
{
	sideOnMove = config.sideOnMove;
	castlingRights = config.castlingRights;
	enPassantLine = config.enPassantLine;
	enPassanter1 = config.enPassanter1;
	enPassanter2 = config.enPassanter2;
	moveNumber = config.moveNumber;
	halfMoves = config.halfMoves;
}



// Handling piece placement
void BoardConfig::updatePiecePlacement(Side side, PieceType type, const Square& oldPos, const Square& newPos)
{
	if (type != KING && type != PAWN)	// != PAWN ???
	{
		SquaresVec& squares = piecePositions[side][type];
		if (oldPos.x < 8 && newPos.x < 8)
			*(std::find(squares.begin(), squares.end(), oldPos)) = newPos;
		else if (oldPos.x < 8)
			squares.erase(std::find(squares.begin(), squares.end(), oldPos));
		else if (newPos.x < 8)
			squares.emplace_back(newPos);
	}
}

void BoardConfig::removePiece(Piece* piece, const Square& nullSquare)
{
	int pieceID = piece->getID();
	const Square& piecePos = piece->getPosition();
	boardProgress.registerChange(new PlacementChange(piece->getID(), piecePos));
	board[piecePos.y][piecePos.x] = nullptr;
	piece->setState(false);
	if (pinDirections[pieceID].x < 8)
	{
		pinDirections[pieceID].x = 8;
		vector<int>& pins = pinnedPieces[piece->getColor()];
		pins.erase(find(pins.begin(), pins.end(), pieceID));
	}
	updatePiecePlacement(piece->getColor(), piece->getType(), piecePos, nullSquare);
	updateObserversByMove(pieceID, piecePos, nullSquare);
	zobristHash.updateHash(piece->getColor(), piece->getType(), piecePos);
}

void BoardConfig::placePiece(int pieceID, PieceType targetType, const Square& newPos, bool updateObserversFlag)
{
	Piece* piece = &pieces[pieceID];
	Side side{ piece->getColor() };
	Square oldPos{piece->getPosition() };
	if (board[oldPos.y][oldPos.x] == piece)
	{
		board[oldPos.y][oldPos.x] = nullptr;
		zobristHash.updateHash(side, piece->getType(), oldPos);
	}
	else
		oldPos.x = 8;
	piece->setPosition(newPos);
	piece->setState(true);
	board[newPos.y][newPos.x] = piece;
	if (targetType != piece->getType())
	{
		updatePiecePlacement(side, piece->getType(), oldPos, BoardManipulation::nullSquare);
		piece->setType(targetType);
		updatePiecePlacement(side, targetType, BoardManipulation::nullSquare, newPos);
		if (moveGenerator != nullptr)
		{
			moveGenerator->removeMoves(pieceID);
			moveGenerator->configureMoveList(pieceID);
		}
	}
	else
		updatePiecePlacement(side, targetType, oldPos, newPos);
	zobristHash.updateHash(side, piece->getType(), newPos);
	if (updateObserversFlag)
	{
		updatePinsDynamically(piece, oldPos, newPos);
		updateObserversByMove(pieceID, oldPos, newPos);
	}
}



// Handling special moves
void BoardConfig::updateMoveCount(const Piece* movedPiece, bool captureFlag)
{
	boardProgress.registerChange(new MoveCountChange(halfMoves, moveNumber));
	if (movedPiece->getColor() == BLACK)
		moveNumber += 1;
	if (captureFlag || movedPiece->getType() == PAWN)
		halfMoves = 0;
	else
		halfMoves += 1;
}

void BoardConfig::updateSideOnMove()
{
	sideOnMove = opposition[sideOnMove];
	zobristHash.updateHashBySideToMoveChange();
}

void BoardConfig::setCastlingRight(Side side, CastleType castleType, bool updatedRight)
{
	if (castlingRights[castlingRightsMapping[side][castleType]] != updatedRight)
	{
		castlingRights[castlingRightsMapping[side][castleType]] = updatedRight;
		zobristHash.updateHash(side, castleType);
	}
}

void BoardConfig::updateCastlingRights(const Piece* movedPiece, const Square& oldPos)
{
	std::bitset<4> oldRights(castlingRights);
	PieceType pieceType{ movedPiece->getType() };
	Side side{ movedPiece->getColor() };
	if (pieceType == KING)
	{
		setCastlingRight(side, SHORT_CASTLE, false);
		setCastlingRight(side, LONG_CASTLE, false);
	}
	else if (pieceType == ROOK)
	{
		if (oldPos == rookCastlingSquares[side][SHORT_CASTLE])
			setCastlingRight(side, SHORT_CASTLE, false);
		else if (oldPos == rookCastlingSquares[side][LONG_CASTLE])
			setCastlingRight(side, LONG_CASTLE, false);
	}
	if (oldRights != castlingRights)
		boardProgress.registerChange(new CastlingChange(oldRights));
}

void BoardConfig::registerEnPassantChange(int enPassanter)
{
	const Piece* piece = &pieces[enPassanter];
	boardProgress.registerChange(new PlacementChange(enPassanter, piece->getPosition()));
	if (moveGenerator != nullptr)
		moveGenerator->generatePieceMoves(piece, getSquaresCoveringChecks(piece->getColor()));
}

void BoardConfig::setEnPassantLine(int line)
{
	if (enPassantLine >= 0) zobristHash.updateHash(enPassantLine);
	if (line >= 0) zobristHash.updateHash(line);
	enPassantLine = line;
}

void BoardConfig::updateEnPassant(const Piece* movedPiece, const Square& oldPos, const Square& newPos)
{
	int oldEnPassant = enPassantLine;
	int enPassanter1tmp = -1;
	int enPassanter2tmp = -1;
	int newEnPassantLine = -2;
	Side side{ movedPiece->getColor() };
	if (movedPiece->getType() == PAWN && pawnMoveDistance[side](oldPos, newPos) == 2)
	{
		newEnPassantLine = newPos.x;
		if (newPos.x != 0)
		{
			Piece* onLeft = board[newPos.y][newPos.x - 1];
			if (onLeft != nullptr && onLeft->getType() == PAWN && onLeft->getColor() != side)
				enPassanter1tmp = onLeft->getID();
		}
		if (newPos.x + 1 < 8)
		{
			Piece* onRight = board[newPos.y][newPos.x + 1];
			if (onRight != nullptr && onRight->getType() == PAWN && onRight->getColor() != side)
				enPassanter2tmp = onRight->getID();
		}
	}
	if (oldEnPassant != newEnPassantLine)
	{
		setEnPassantLine(newEnPassantLine);
		if (enPassanter1 > 0)
			registerEnPassantChange(enPassanter1);
		else if (enPassanter1tmp > 0)
			registerEnPassantChange(enPassanter1tmp);
		if (enPassanter2 > 0)
			registerEnPassantChange(enPassanter2);
		else if (enPassanter2tmp > 0)
			registerEnPassantChange(enPassanter2tmp);
		boardProgress.registerChange(new EnPassantChange(oldEnPassant, enPassanter1, enPassanter2));
		enPassanter1 = enPassanter1tmp;
		enPassanter2 = enPassanter2tmp;
	}
}



// Pin handlers
Piece* BoardConfig::nextInDirection(const Square& start, const Direction& moveDir, SearchCondition searchCondition)
{
	Square square(start + moveDir);
	while (searchCondition(square))
	{
		Piece* pieceOnPath = board[square.y][square.x];
		if (pieceOnPath != nullptr)
			return pieceOnPath;
		square += moveDir;
	}
	return nullptr;
}

void BoardConfig::clearPins()
{
	for (const int& pinnedPieceID : pinnedPieces[WHITE])
		pinDirections[pinnedPieceID].x = 8;
	pinnedPieces[WHITE].clear();
	for (const int& pinnedPieceID : pinnedPieces[BLACK])
		pinDirections[pinnedPieceID].x = 8;
	pinnedPieces[BLACK].clear();
}

void BoardConfig::removePin(Side side, const Piece* foundPiece, int foundPieceID)
{
	pinnedPieces[side].erase(std::find(pinnedPieces[side].begin(), pinnedPieces[side].end(), foundPieceID));
	pinDirections[foundPieceID].x = 8;
	if (moveGenerator != nullptr)
		moveGenerator->generatePieceMoves(foundPiece, getSquaresCoveringChecks(foundPiece->getColor()));
}

void BoardConfig::searchForPins(Side side, const Square& kingPos, const Square& dir)
{
	Piece* firstFind = nextInDirection(kingPos, dir, BoardManipulation::isCorrectSquare);
	if (firstFind != nullptr && firstFind->getColor() == side)
	{
		int firstID = firstFind->getID();
		Piece* secondFind = nextInDirection(firstFind->getPosition(), dir, BoardManipulation::isCorrectSquare);
		if (secondFind == nullptr)
		{
			if (pinDirections[firstID].x < 8)
				removePin(side, firstFind, firstID);
			return;
		}
		int secondID = secondFind->getID();
		if (secondFind->getColor() == side)
		{
			if (pinDirections[firstID].x < 8)
				removePin(side, firstFind, firstID);
			else if (pinDirections[secondID].x < 8)
				removePin(side, secondFind, secondID);
		}
		else
		{
			bool isRookDir = dir.x == 0 || dir.y == 0;
			if ((isRookDir && secondFind->hasRookAbilities()) || (!isRookDir && secondFind->hasBishopAbilities()))
			{
				if (pinDirections[firstID].x >= 8)
				{
					// save pin
					pinnedPieces[side].push_back(firstID);
					pinDirections[firstID] = dir;
					if (moveGenerator != nullptr)
						moveGenerator->generatePieceMoves(firstFind, getSquaresCoveringChecks(side));
				}

			}
			else if (pinDirections[firstID].x < 8)
				removePin(side, firstFind, firstID);
		}
	}
}

void BoardConfig::updatePinsFromKing(Side side, const Square& square)
{
	const Square& kingPos = getKing(side)->getPosition();
	const Direction& dir = BoardManipulation::toDirectionalVector(kingPos, square);
	if (dir.x < 8)
		searchForPins(side, kingPos, dir);
}

void BoardConfig::updatePinsFromKing(Side side)
{
	const Square& kingPos = getKing(side)->getPosition();
	const DirectionsVec& queenDirs = PieceHandlers::getMoveDirections(QUEEN);
	for (const Direction& dir : queenDirs)
		searchForPins(side, kingPos, dir);
}

void BoardConfig::updatePinsFromSquare(const Square& square)
{
	updatePinsFromKing(WHITE, square);
	updatePinsFromKing(BLACK, square);
}

void BoardConfig::updatePinsStatically()
{
	// CHANGED
	clearPins();
	updatePinsFromKing(WHITE);
	updatePinsFromKing(BLACK);
}

void BoardConfig::updatePinsDynamically(const Piece* piece, const Square& oldPos, const Square& newPos)
{
	Side side = piece->getColor();
	if (piece->getType() == KING)
	{
		Side opposite = opposition[side];
		updatePinsFromKing(opposite, oldPos);
		updatePinsFromKing(opposite, newPos);
		for (int& id : pinnedPieces[side])
		{
			pinDirections[id].x = 8;
			if (moveGenerator != nullptr)
				moveGenerator->generatePieceMoves(&pieces[id], getSquaresCoveringChecks(pieces[id].getColor()));
		}
		pinnedPieces[side].clear();
		updatePinsFromKing(WHITE);
	}
	else
	{
		if (pinDirections[piece->getID()].x < 8)
			removePin(side, piece, piece->getID());
		if (oldPos.x < 8)
			updatePinsFromSquare(oldPos);
		if (newPos.x < 8)
			updatePinsFromSquare(newPos);
	}
}



// Check handlers
void BoardConfig::clearChecks()
{
	checkers[WHITE][0] = checkers[WHITE][1] = nullptr;
	checkers[BLACK][0] = checkers[BLACK][1] = nullptr;
	safeMoves[WHITE] = safeMoves[BLACK] = nullptr;
}

void BoardConfig::updateChecksInDirection(const Piece* king, const Direction& dir, AttackingCondition attackingCondition, int& checkCount)
{
	const Square& kingPos = king->getPosition();
	Piece* firstFind = nextInDirection(kingPos, dir, BoardManipulation::isCorrectSquare);
	if (firstFind != nullptr && firstFind->getColor() != king->getColor() && attackingCondition(firstFind))
	{
		checkers[firstFind->getColor()][checkCount] = firstFind;
		checkCount += 1;
		if (checkCount == 1)
			safeMoves[king->getColor()] = &BoardManipulation::checkSavers[firstFind->getPosition().x][firstFind->getPosition().y][kingPos.x + dir.x][kingPos.y + dir.y];
		else
			safeMoves[king->getColor()] = BoardManipulation::nullCheckSaver;
	}
}

void BoardConfig::updateChecksInDirection(const Piece* king, const Direction& dir, int& checkCount)
{
	if (dir.x == 0 || dir.y == 0)
		updateChecksInDirection(king, dir, rookAttackCondition, checkCount);
	else
		updateChecksInDirection(king, dir, bishopAttackCondition, checkCount);
}

void BoardConfig::updateChecksInDirections(const Piece* king, const DirectionsVec& dirs, AttackingCondition attackingCondition, int& checkCount)
{
	for (const Direction& dir : dirs)
		updateChecksInDirection(king, dir, attackingCondition, checkCount);
}

void BoardConfig::updateChecksForKnight(Piece* knight, Side checkedSide, const Square& kingPos, int& checkCount)
{
	const Square& knightPos = knight->getPosition();
	if (isKnightAttackingSquare(knightPos, kingPos))
	{
		checkers[knight->getColor()][checkCount] = knight;
		checkCount += 1;
		safeMoves[checkedSide] = checkCount == 1 ? &BoardManipulation::checkSavers[knightPos.x][knightPos.y][knightPos.x][knightPos.y] : BoardManipulation::nullCheckSaver;
	}
}

void BoardConfig::updateChecksForPawn(const Square& square, const Square& kingPos, Side kingColor, int& checkCount)
{
	Piece* onLeft = board[square.y][square.x];
	if (BoardManipulation::isCorrectSquare(square) && onLeft != nullptr && onLeft->getType() == PAWN && onLeft->getColor() != kingColor)
	{
		checkers[opposition[kingColor]][checkCount] = onLeft;
		checkCount += 1;
		if (checkCount == 1)
			safeMoves[kingColor] = &BoardManipulation::checkSavers[square.x][square.y][square.x][square.y];
		else
			safeMoves[kingColor] = BoardManipulation::nullCheckSaver;
	}
}

void BoardConfig::updateChecksForSide(Side side)
{
	const Piece* king = getKing(side);
	const Square& kingPos = king->getPosition();
	int checkCount = 0;
	updateChecksInDirections(king, PieceHandlers::getMoveDirections(BISHOP), bishopAttackCondition, checkCount);
	updateChecksInDirections(king, PieceHandlers::getMoveDirections(ROOK), rookAttackCondition, checkCount);
	Side opposite = opposition[side];
	for (const Square& knightPos : piecePositions[opposite][KNIGHT])
		updateChecksForKnight(board[knightPos.y][knightPos.x], side, kingPos, checkCount);
	updateChecksForPawn(Square(kingPos.x - 1, kingPos.y + pawnFrontNum[side]), kingPos, side, checkCount);
	updateChecksForPawn(Square(kingPos.x + 1, kingPos.y + pawnFrontNum[side]), kingPos, side, checkCount);
}

void BoardConfig::updateChecksStatically()
{
	clearChecks();
	updateChecksForSide(WHITE);
	updateChecksForSide(BLACK);
}

void BoardConfig::updateChecksDynamically(Piece* piece, const Square& oldPos, const Square& newPos)
{
	boardProgress.registerChange(new CheckerChange(checkers, safeMoves));
	Side side = piece->getColor();
	Side opposite = opposition[side];
	checkers[opposite][0] = checkers[opposite][1] = nullptr;
	safeMoves[side] = nullptr;
	int checkCount = 0;
	const Piece* opponentKing = getKing(opposite);
	const Direction& dirToOldPos = BoardManipulation::toDirectionalVector(opponentKing->getPosition(), oldPos);
	if (dirToOldPos.x < 8)
		updateChecksInDirection(opponentKing, dirToOldPos, checkCount);
	const Direction& dirToNewPos = BoardManipulation::toDirectionalVector(opponentKing->getPosition(), newPos);
	if (piece->getType() == KNIGHT)
		updateChecksForKnight(piece, opposite, opponentKing->getPosition(), checkCount);
	else if (piece->getType() == PAWN && isPawnAttackingSquare(side, newPos, opponentKing->getPosition()))
	{
		checkers[side][checkCount] = piece;
		checkCount += 1;
		if (checkCount == 1)
			safeMoves[opposite] = &BoardManipulation::checkSavers[newPos.x][newPos.y][newPos.x][newPos.y];
		else
			safeMoves[opposite] = BoardManipulation::nullCheckSaver;
	}
	else if (dirToNewPos.x < 8)
		updateChecksInDirection(opponentKing, dirToNewPos, checkCount);
}
