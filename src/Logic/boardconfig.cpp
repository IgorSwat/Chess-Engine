#include "movegenerator.h"
#include <exception>
#include <iostream>
#include <algorithm>

using sf::Vector2i;
using std::vector;
using std::find;

Vector2i BoardConfig::directionalVectors[15][15] {};
vector<Vector2i> BoardConfig::checkSavers[8][8][8][8] {};
vector<Vector2i> BoardConfig::emptySaver {};
bool BoardConfig::directionalComputed = false;
bool BoardConfig::saversComputed = false;


void BoardConfig::initDirectionalVectors()
{
    if (!directionalComputed)
    {
        for (int x = -7; x < 8; x++)
        {
            for (int y = -7; y < 8; y++)
                directionalVectors[7 - x][7 - y] = toDirectionalVector(Vector2i(x, y));
        }
        directionalComputed = true;
    }
}

void BoardConfig::initCheckSavers()
{
    if (!saversComputed)
    {
        for (int x1 = 0; x1 < 8; x1++)
        {
            for (int y1 = 0; y1 < 8; y1++)
            {
                for (int x2 = 0; x2 < 8; x2++)
                {
                    for (int y2 = 0; y2 < 8; y2++)
                    {
                        Vector2i p1(x1, y1);
                        Vector2i p2(x2, y2);
                        if (p1 == p2)
                            checkSavers[x1][y1][x2][y2].push_back(p1);
                        else
                        {
                            Vector2i dir = toDirectionalVector(p1, p2);
                            if (dir.x < 8)
                            {
                                Vector2i currPos = p1;
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
        saversComputed = true;
    }
}

Vector2i BoardConfig::toDirectionalVector(const sf::Vector2i& mv)
{
    if (mv.x == 0 && mv.y == 0)
        return Vector2i(8, 8);
    int x_uns = abs(mv.x);
    int y_uns = abs(mv.y);
    if (mv.x == 0)
        return Vector2i(mv.x, mv.y / y_uns);
    if (mv.y == 0)
        return Vector2i(mv.x / x_uns, mv.y);
    if (x_uns == y_uns)
        return Vector2i(mv.x / x_uns, mv.y / y_uns);
    return Vector2i(8, 8);
}

inline Vector2i BoardConfig::toDirectionalVector(const sf::Vector2i& initPos, const sf::Vector2i& targetPos)
{
    return toDirectionalVector(targetPos - initPos);
}

inline const Vector2i& BoardConfig::toDirectionalVector2(const sf::Vector2i& initPos, const sf::Vector2i& targetPos)
{
    return toDirectionalVector2(targetPos - initPos);
}



void BoardConfig::placePiece(const Vector2i& pos, int pieceID, bool observableFlag)
{
    Piece* piece = pieces[pieceID];
    Vector2i oldPos = pieces[pieceID]->getPos();
    if (table[oldPos.y][oldPos.x] == piece)
        table[oldPos.y][oldPos.x] = nullptr;
    else
        oldPos = Vector2i(8,8);
    table[pos.y][pos.x] = pieces[pieceID];
    pieces[pieceID]->setPos(pos);
    pieces[pieceID]->setState(true);
    if (observableFlag)
    {
        updatePinsDynamic(pieceID, oldPos, pos);
        updateObserversByMove(pieceID, oldPos, pos);
    }
}

void BoardConfig::placePiece(const sf::Vector2i& pos, int pieceID, PieceType type, bool observableFlag)
{
    Piece* piece = pieces[pieceID];
    if (piece->getType() != type)
    {
        COLOR color = piece->getColor();
        Vector2i oldPos = pieces[pieceID]->getPos();
        if (table[oldPos.y][oldPos.x] == piece)
            table[oldPos.y][oldPos.x] = nullptr;
        else
            oldPos = Vector2i(8, 8);
        Vector2i posCopied(pos);
        delete pieces[pieceID];
        switch (type)
        {
        case PieceType::PAWN:
            pieces[pieceID] = new Pawn(color, pieceID, posCopied);
            break;
        case PieceType::KNIGHT:
            pieces[pieceID] = new Knight(color, pieceID, posCopied);
            break;
        case PieceType::BISHOP:
            pieces[pieceID] = new Bishop(color, pieceID, posCopied);
            break;
        case PieceType::ROOK:
            pieces[pieceID] = new Rook(color, pieceID, posCopied);
            break;
        case PieceType::QUEEN:
            pieces[pieceID] = new Queen(color, pieceID, posCopied);
            break;
        default:
            throw std::bad_exception();
        }
        table[posCopied.y][posCopied.x] = pieces[pieceID];
        if (generator != nullptr)
        {
            generator->removeMoves(pieceID);
            generator->configureMoveList(pieceID);
        }
        if (observableFlag)
        {
            updatePinsDynamic(pieceID, oldPos, posCopied);
            updateObserversByMove(pieceID, oldPos, posCopied);
        }
    }
    else
        placePiece(pos, pieceID, observableFlag);
}

void BoardConfig::clearAttacksTable()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            attacksTable[i][j][0] = 0;
            attacksTable[i][j][1] = 0;
        }
    }
}

void BoardConfig::clearBoard()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
            table[i][j] = nullptr;
    }
    for (int i = 0; i < 32; i++)
    {
        if (pieces[i] != nullptr)
            pieces[i]->setState(false);
    }
    resetPins();
    whiteCkecker[0] = whiteCkecker[1] = nullptr;
    blackCkecker[0] = blackCkecker[1] = nullptr;
    safeMoves[0] = safeMoves[1] = nullptr;
    while (!boardProgress.isEmpty())
        boardProgress.getLatestChanges();
}



Piece* BoardConfig::nextInDirection(const Vector2i& start, const Vector2i& mv, Piece* except)
{
    Vector2i curr = start + mv;
    while (isCorrectSquare(curr))
    {
        Piece* onPath = table[curr.y][curr.x];
        if (onPath != nullptr && onPath != except)
            return onPath;
        curr = curr + mv;
    }
    return nullptr;
}

Piece* BoardConfig::nextInDirection(const sf::Vector2i& start, const sf::Vector2i& mv, const sf::Vector2i& end, Piece* except)
{
    Vector2i curr = start + mv;
    while (curr != end)
    {
        Piece* onPath = table[curr.y][curr.x];
        if (onPath != nullptr && onPath != except)
            return onPath;
        curr = curr + mv;
    }
    return nullptr;
}

bool BoardConfig::follows(const sf::Vector2i& pos, const sf::Vector2i& mv, const sf::Vector2i& target)
{
    Vector2i diffPos(0, 0);
    if (mv.x != 0)
        diffPos.x = (target.x - pos.x) / mv.x;
    if (mv.y != 0)
        diffPos.y = (target.y - pos.y) / mv.y;
    if (diffPos.x > 0 || diffPos.y > 0)
        return true;
    return false;
}




void BoardConfig::searchForPins(Piece* king, const Vector2i& mv)
{
    int sideID = (int)king->getColor();
    Piece* firstFind = nextInDirection(king->getPos(), mv, nullptr);
    if (firstFind != nullptr && firstFind->getColor() == king->getColor())
    {
        Piece* secondFind = nextInDirection(firstFind->getPos(), mv, nullptr);
        int firstID = firstFind->getID();
        if (secondFind == nullptr)
        {
            if (pinVectors[firstID].x < 8)
            {
                pinnedPieces[sideID].erase(find(pinnedPieces[sideID].begin(), pinnedPieces[sideID].end(), firstID));
                pinVectors[firstID].x = 8;
                if (generator != nullptr)
                    generator->generatePieceMoves(firstFind, getSquaresCoveringChecks(firstFind->getColor()));
            }
            return;
        }
        int secondID = secondFind->getID();
        if (secondFind->getColor() == king->getColor())
        {
            if (pinVectors[firstID].x < 8)
            {
                pinnedPieces[sideID].erase(find(pinnedPieces[sideID].begin(), pinnedPieces[sideID].end(), firstID));
                pinVectors[firstID].x = 8;
                if (generator != nullptr)
                    generator->generatePieceMoves(firstFind, getSquaresCoveringChecks(firstFind->getColor()));
            }
            else if (pinVectors[secondID].x < 8)
            {
                pinnedPieces[sideID].erase(find(pinnedPieces[sideID].begin(), pinnedPieces[sideID].end(), secondID));
                pinVectors[secondID].x = 8;
                if (generator != nullptr)
                    generator->generatePieceMoves(secondFind, getSquaresCoveringChecks(secondFind->getColor()));
            }
        }
        else
        {
            bool isRookDir = mv.x == 0 || mv.y == 0;
            if ((isRookDir && secondFind->hasRookAbilities()) || (!isRookDir && secondFind->hasBishopAbilities()))
            {
                if (pinVectors[firstID].x >= 8)
                {
                    pinnedPieces[sideID].push_back(firstID);
                    pinVectors[firstID] = mv;
                    if (generator != nullptr)
                        generator->generatePieceMoves(firstFind, getSquaresCoveringChecks(firstFind->getColor()));
                }
            }
            else if (pinVectors[firstID].x < 8)
            {
                pinnedPieces[sideID].erase(find(pinnedPieces[sideID].begin(), pinnedPieces[sideID].end(), firstID));
                pinVectors[firstID].x = 8;
                if (generator != nullptr)
                    generator->generatePieceMoves(firstFind, getSquaresCoveringChecks(firstFind->getColor()));
            }
        }
    }
}

inline void BoardConfig::updatePinsFromKing(Piece* king, const Vector2i& square)
{
    const Vector2i& mv = toDirectionalVector2(king->getPos(), square);
    if (mv.x < 8)
        searchForPins(king, mv);
}

inline void BoardConfig::updatePinsFromSquare(const Vector2i& square)
{
    updatePinsFromKing(whiteKing, square);
    updatePinsFromKing(blackKing, square);
}

void BoardConfig::resetPins()
{
    for (int i = 0; i < 32; i++)
        pinVectors[i] = Vector2i(8,8);
    pinnedPieces[0].clear();
    pinnedPieces[1].clear();
}

Vector2i BoardConfig::isPinned(Piece* piece)
{
    King* king;
    if (piece->getColor() == COLOR::WHITE)
        king = whiteKing;
    else
        king = blackKing;
    const Vector2i& directionalVector = toDirectionalVector2(king->getPos(), piece->getPos());
    if (directionalVector.x >= 8)
        return directionalVector;
    Piece* found = nextInDirection(king->getPos(), directionalVector, piece);
    if (found == nullptr || found->getColor() == king->getColor())
        return Vector2i(8, 8);
    if (!found->isAttacking(king->getPos()))
        return Vector2i(8, 8);
    return directionalVector;
}



void BoardConfig::updateSideOnMove()
{
    boardProgress.registerChange(new SideOnMoveChange(sideOnMove));
    sideOnMove = ++sideOnMove;
}

void BoardConfig::updateMoveCounts(Piece* movedPiece, bool captureFlag)
{
    boardProgress.registerChange(new MoveCountChange(halfMoves, moveNumber));
    if (movedPiece->getColor() == COLOR::BLACK)
        moveNumber++;
    if (captureFlag || isPawn(movedPiece))
        halfMoves = 0;
    else
        halfMoves++;
}

void BoardConfig::updateEnPassant(Piece* piece, bool captureFlag, const sf::Vector2i& oldPos, const sf::Vector2i& newPos)
{
    int oldEnPassant = enPassantLine;
    int enPassanter1tmp = -1;
    int enPassanter2tmp = -1;
    if (captureFlag || !isPawn(piece) || abs(newPos.y - oldPos.y) != 2)
        enPassantLine = -2;
    else
    {
        enPassantLine = newPos.x;
        COLOR side = piece->getColor();
        if (newPos.x - 1 >= 0)
        {
            Piece* onLeft = table[newPos.y][newPos.x - 1];
            if (onLeft != nullptr && isPawn(onLeft) && onLeft->getColor() != side)
                enPassanter1tmp = onLeft->getID();
        }
        if (newPos.x + 1 < 8)
        {
            Piece* onRight = table[newPos.y][newPos.x + 1];
            if (onRight != nullptr && isPawn(onRight) && onRight->getColor() != side)
                enPassanter2tmp = onRight->getID();
        }
    }
    if (oldEnPassant != enPassantLine)
    {
        if (enPassanter1 > 0)
        {
            const Vector2i& pos = getPiece(enPassanter1)->getPos();
            boardProgress.registerChange(new PlacementChange(enPassanter1, Vector2i(pos)));
            if (generator != nullptr)
                generator->generatePieceMoves(pieces[enPassanter1], getSquaresCoveringChecks(pieces[enPassanter1]->getColor()));
        }
        else if (enPassanter1tmp > 0)
        {
            const Vector2i& pos = getPiece(enPassanter1tmp)->getPos();
            boardProgress.registerChange(new PlacementChange(enPassanter1tmp, Vector2i(pos)));
            if (generator != nullptr)
                generator->generatePieceMoves(pieces[enPassanter1tmp], getSquaresCoveringChecks(pieces[enPassanter1]->getColor()));
        }
        if (enPassanter2 > 0)
        {
            const Vector2i& pos = getPiece(enPassanter2)->getPos();
            boardProgress.registerChange(new PlacementChange(enPassanter2, Vector2i(pos)));
            if (generator != nullptr)
                generator->generatePieceMoves(pieces[enPassanter2], getSquaresCoveringChecks(pieces[enPassanter1]->getColor()));
        }
        else if (enPassanter2tmp > 0)
        {
            const Vector2i& pos = getPiece(enPassanter2tmp)->getPos();
            boardProgress.registerChange(new PlacementChange(enPassanter2tmp, Vector2i(pos)));
            if (generator != nullptr)
                generator->generatePieceMoves(pieces[enPassanter2tmp], getSquaresCoveringChecks(pieces[enPassanter1]->getColor()));
        }
        boardProgress.registerChange(new EnPassantChange(oldEnPassant, enPassanter1, enPassanter2));
        enPassanter1 = enPassanter1tmp;
        enPassanter2 = enPassanter2tmp;
    }
}

void BoardConfig::updateCastling(Piece* piece, const sf::Vector2i& oldPos)
{
    bool Ws {Wshort}, Wl {Wlong}, Bs {Bshort}, Bl {Blong};
    if (piece->getColor() == COLOR::WHITE)
    {
        if (isKing(piece))
            Wshort = Wlong = false;
        else if (isRook(piece) && oldPos == Vector2i(0, 7))
            Wlong = false;
        else if (isRook(piece) && oldPos == Vector2i(7, 7))
            Wshort = false;
    }
    else
    {
        if (isKing(piece))
            Bshort = Blong = false;
        else if (isRook(piece) && oldPos == Vector2i(0, 0))
            Blong = false;
        else if (isRook(piece) && oldPos == Vector2i(7, 0))
            Bshort = false;
    }
    if (Ws != Wshort || Wl != Wlong || Bs != Bshort || Bl != Blong)
        boardProgress.registerChange(new CastlingChange(Ws, Wl, Bs, Bl));
}

void BoardConfig::updateChecks(Piece* movedPiece, const Vector2i& oldPos)
{
    King* king;
    Piece** checker;
    Piece* oldWhiteCheckers[2] {whiteCkecker[0], whiteCkecker[1]};
    Piece* oldBlackCheckers[2] {blackCkecker[0], blackCkecker[1]};
    vector<Vector2i>* oldSafeMoves[2] {safeMoves[0], safeMoves[1]};
    if (movedPiece->getColor() == COLOR::WHITE) {
        king = blackKing;
        checker = whiteCkecker;
        blackCkecker[0] = blackCkecker[1] = nullptr;
        safeMoves[0] = nullptr;
    }
    else
    {
        king = whiteKing;
        checker = blackCkecker;
        whiteCkecker[0] = whiteCkecker[1] = nullptr;
        safeMoves[1] = nullptr;
    }
    int checkedSideID = (int)king->getColor();
    int checkCount = 0;
    const Vector2i& kingPos = king->getPos();
    const Vector2i& mv = toDirectionalVector2(movedPiece->getPos(), king->getPos());
    if (isKnight(movedPiece) && movedPiece->isReachable(kingPos)) {
        const Vector2i& pos = movedPiece->getPos();
        checker[0] = movedPiece;
        safeMoves[checkedSideID] = &checkSavers[pos.x][pos.y][pos.x][pos.y];
        checkCount += 1;
    }
    else if (mv.x < 8)
    {
        if (movedPiece->isAttacking(kingPos) && nextInDirection(movedPiece->getPos(), mv, nullptr) == king)
        {
            const Vector2i& pos = movedPiece->getPos();
            checker[0] = movedPiece;
            safeMoves[checkedSideID] = &checkSavers[pos.x][pos.y][kingPos.x - mv.x][kingPos.y - mv.y];
            checkCount += 1;
        }
    }
    const Vector2i& mv1 = toDirectionalVector2(kingPos, oldPos);
    const Vector2i& mv2 = toDirectionalVector2(kingPos, movedPiece->getPos());
    if (mv1.x < 8 && mv1 != mv2)
    {
        Piece* found = nextInDirection(kingPos, mv1, movedPiece);
        if (found != nullptr && found->getColor() != king->getColor() && found->isAttacking(kingPos))
        {
            const Vector2i& pos = found->getPos();
            checker[checkCount] = found;
            checkCount += 1;
            if (checkCount == 1)
                safeMoves[checkedSideID] = &checkSavers[pos.x][pos.y][kingPos.x + mv1.x][kingPos.y + mv1.y];
            else
                safeMoves[checkedSideID] = &emptySaver;
        }
    }
    if (whiteCkecker[0] != oldWhiteCheckers[0] || blackCkecker[0] != oldBlackCheckers[0])
        boardProgress.registerChange(new CheckerChange(oldWhiteCheckers, oldBlackCheckers, oldSafeMoves));
}

void BoardConfig::updatePinsDynamic(int pieceID, const sf::Vector2i& oldPos, const sf::Vector2i& newPos)
{
    if (pieceID == 0 || pieceID == 16)
    {
        Piece* king = pieces[pieceID];
        Piece* oppositeKing = pieces[(pieceID + 16) % 32];
        int sideID = (int)king->getColor();
        updatePinsFromKing(oppositeKing, oldPos);
        updatePinsFromKing(oppositeKing, newPos);
        for (const int& id : pinnedPieces[sideID])
        {
            pinVectors[id].x = 8;
            if (generator != nullptr)
                generator->generatePieceMoves(pieces[id], getSquaresCoveringChecks(pieces[id]->getColor()));
        }
        pinnedPieces[sideID].clear();
        const dirVector& dirs = Queen::directions();
        for (const Vector2i& dir : dirs)
            searchForPins(king, dir);
    }
    else
    {
        if (pinVectors[pieceID].x < 8)
        {
            int sideID = pieceID < 16 ? 0 : 1;
            pinVectors[pieceID].x = 8;
            pinnedPieces[sideID].erase(find(pinnedPieces[sideID].begin(), pinnedPieces[sideID].end(), pieceID));
            if (generator != nullptr)
                generator->generatePieceMoves(pieces[pieceID], getSquaresCoveringChecks(pieces[pieceID]->getColor()));
        }
        if (oldPos.x < 8)
            updatePinsFromSquare(oldPos);
        if (newPos.x < 8)
            updatePinsFromSquare(newPos);
    }
}

void BoardConfig::updatePinsStatic()
{
    for (int i = 0; i < 32; i++)
    {
        Piece* piece = pieces[i];
        Vector2i pinDir;
        if (piece->isActive() && (pinDir = isPinned(piece)).x < 8)
        {
            pinVectors[i] = pinDir;
            pinnedPieces[i / 16].push_back(i);
        }
    }
}

void BoardConfig::updateChecksForSide(COLOR side)
{

    Piece* king;
    Piece** checker;
    int checkedSideID;
    if (side == COLOR::WHITE)
    {
        king = whiteKing;
        checker = blackCkecker;
        checkedSideID = 0;
    }
    else
    {
        king = blackKing;
        checker = whiteCkecker;
        checkedSideID = 1;
    }
    int checkCount = 0;
    const dirVector& dirs = Queen::directions();
    for (const Vector2i& dir : dirs)
    {
        Piece* firstFind = nextInDirection(king->getPos(), dir, nullptr);
        if (firstFind != nullptr && firstFind->getColor() != side && firstFind->isAttacking(king->getPos()))
        {
            const Vector2i& pos = firstFind->getPos();
            checker[checkCount] = firstFind;
            checkCount += 1;
            if (checkCount == 1)
                safeMoves[checkedSideID] = &checkSavers[pos.x][pos.y][king->getPos().x + dir.x][king->getPos().y + dir.y];
            else
                safeMoves[checkedSideID] = &emptySaver;
        }
    }
    const dirVector& knightDirs = Knight::directions();
    for (const Vector2i& dir : knightDirs)
    {
        Vector2i pos = king->getPos() + dir;
        if (!isCorrectSquare(pos))
            continue;
        Piece* firstFind = getPiece(pos);
        if (firstFind != nullptr && firstFind->getColor() != side && firstFind->isAttacking(king->getPos()))
        {
            checker[checkCount] = firstFind;
            checkCount += 1;
            if (checkCount == 1)
                safeMoves[checkedSideID] = &checkSavers[pos.x][pos.y][pos.x][pos.y];
            else
                safeMoves[checkedSideID] = &emptySaver;
        }
    }
}

inline void BoardConfig::updateChecksStatic()
{
    updateChecksForSide(COLOR::WHITE);
    updateChecksForSide(COLOR::BLACK);
}



// Constructors and static setters
BoardConfig::BoardConfig()
{
    initDirectionalVectors();
    initCheckSavers(); // Done
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            table[i][j] = nullptr;
            attacksTable[i][j][0] = 0;
            attacksTable[i][j][1] = 0;
        }
    }

    whiteKing = new King(COLOR::WHITE, 0, Vector2i(4, 7));
    pieces[0] = whiteKing;
    for (int i = 0; i < 8; i++)
        pieces[1 + i] = new Pawn(COLOR::WHITE, 1 + i, Vector2i(i, 6));
    pieces[9] = new Knight(COLOR::WHITE, 9, Vector2i(1, 7));
    pieces[10] = new Knight(COLOR::WHITE, 10, Vector2i(6, 7));
    pieces[11] = new Bishop(COLOR::WHITE, 11, Vector2i(2, 7));
    pieces[12] = new Bishop(COLOR::WHITE, 12, Vector2i(5, 7));
    pieces[13] = new Rook(COLOR::WHITE, 13, Vector2i(0, 7));
    pieces[14] = new Rook(COLOR::WHITE, 14, Vector2i(7, 7));
    pieces[15] = new Queen(COLOR::WHITE, 15, Vector2i(3, 7));

    blackKing = new King(COLOR::BLACK, blackID, Vector2i(4, 0));
    pieces[blackID] = blackKing;
    for (int i = 0; i < 8; i++)
        pieces[blackID + 1 + i] = new Pawn(COLOR::BLACK, blackID + 1 + i, Vector2i(i, 1));
    pieces[blackID + 9] = new Knight(COLOR::BLACK, blackID + 9, Vector2i(1, 0));
    pieces[blackID + 10] = new Knight(COLOR::BLACK, blackID + 10, Vector2i(6, 0));
    pieces[blackID + 11] = new Bishop(COLOR::BLACK, blackID + 11, Vector2i(2, 0));
    pieces[blackID + 12] = new Bishop(COLOR::BLACK, blackID + 12, Vector2i(5, 0));
    pieces[blackID + 13] = new Rook(COLOR::BLACK, blackID + 13, Vector2i(0, 0));
    pieces[blackID + 14] = new Rook(COLOR::BLACK, blackID + 14, Vector2i(7, 0));
    pieces[blackID + 15] = new Queen(COLOR::BLACK, blackID + 15, Vector2i(3, 0));

    setToDefault();
}

BoardConfig::~BoardConfig()
{
    for (int i = 0; i < 32; i++)
        delete pieces[i];
}

void BoardConfig::setToDefault()
{
    sideOnMove = COLOR::WHITE;
    Wshort = Wlong = Bshort = Blong = true;
    enPassantLine = -2;
    moveNumber = 1;
    halfMoves = 0;

    clearBoard();

    placePiece(Vector2i(4, 7), 0, false);
    for (int i = 0; i < 8; i++)
        placePiece(Vector2i(i, 6), 1 + i, PieceType::PAWN, false);
    placePiece(Vector2i(1, 7), 9, PieceType::KNIGHT, false);
    placePiece(Vector2i(6, 7), 10, PieceType::KNIGHT, false);
    placePiece(Vector2i(2, 7), 11, PieceType::BISHOP, false);
    placePiece(Vector2i(5, 7), 12, PieceType::BISHOP, false);
    placePiece(Vector2i(0, 7), 13, PieceType::ROOK, false);
    placePiece(Vector2i(7, 7), 14, PieceType::ROOK, false);
    placePiece(Vector2i(3, 7), 15, PieceType::QUEEN, false);

    placePiece(Vector2i(4, 0), blackID, false);
    for (int i = 0; i < 8; i++)
        placePiece(Vector2i(i, 1), blackID + 1 + i, PieceType::PAWN, false);
    placePiece(Vector2i(1, 0), blackID + 9, PieceType::KNIGHT, false);
    placePiece(Vector2i(6, 0), blackID + 10, PieceType::KNIGHT, false);
    placePiece(Vector2i(2, 0), blackID + 11, PieceType::BISHOP, false);
    placePiece(Vector2i(5, 0), blackID + 12, PieceType::BISHOP, false);
    placePiece(Vector2i(0, 0), blackID + 13, PieceType::ROOK, false);
    placePiece(Vector2i(7, 0), blackID + 14, PieceType::ROOK, false);
    placePiece(Vector2i(3, 0), blackID + 15, PieceType::QUEEN, false);

    updatePinsStatic();
    resetObservers();
}

bool BoardConfig::setFromFEN(const std::string& FEN)
{
    this->FENreader.setFEN(FEN);
    clearBoard();
    if (this->FENreader.parseToConfig(this))
    {
        updatePinsStatic();
        updateChecksStatic();
        resetObservers();
        return true;
    }
    return false;
}



void BoardConfig::movePiece(int pieceID, const sf::Vector2i& newPos, PieceType promoteTo)
{
    bool captureFlag = false;
    Piece* piece = pieces[pieceID];
    Vector2i oldPos(piece->getPos());
    if (table[newPos.y][newPos.x] != nullptr)
    {
        int id = table[newPos.y][newPos.x]->getID();
        boardProgress.registerChange(new PlacementChange(id, Vector2i(newPos)));
        table[newPos.y][newPos.x]->setState(false);
        captureFlag = true;
        if (pinVectors[id].x < 8)
        {
            pinVectors[id].x = 8;
            vector<int>& pins = pinnedPieces[id < 16 ? 0 : 1];
            pins.erase(find(pins.begin(), pins.end(), id));

        }
        updateObserversByMove(id, newPos, Vector2i(9, 8));
    }
    table[newPos.y][newPos.x] = piece;
    table[oldPos.y][oldPos.x] = nullptr;
    piece->setPos(newPos);
    boardProgress.registerChange(new PlacementChange(pieceID, oldPos));
    if (isPawn(piece) && promoteTo != PieceType::PAWN)
        piece = promote(pieceID, promoteTo);
    updateChecks(piece, oldPos);
    updatePinsDynamic(pieceID, oldPos, newPos);
    updateEnPassant(piece, captureFlag, oldPos, newPos);
    updateCastling(piece, oldPos);
    updateSideOnMove();
    updateMoveCounts(piece, captureFlag);
    boardProgress.pushChanges();
    updateObserversByMove(pieceID, oldPos, newPos);
}

void BoardConfig::castle(Piece* king, const sf::Vector2i& mv)
{
    Vector2i kingPos(king->getPos());
    Piece* rook;
    if (mv.x < 0)
        rook = table[kingPos.y][kingPos.x - 4];
    else
        rook = table[kingPos.y][kingPos.x + 3];
    Vector2i rookPos(rook->getPos());
    table[kingPos.y][kingPos.x + mv.x] = king;
    table[kingPos.y][kingPos.x] = nullptr;
    table[rookPos.y][rookPos.x] = nullptr;
    table[kingPos.y][kingPos.x + mv.x / 2] = rook;
    Vector2i kingTargetPos = kingPos + mv;
    Vector2i rookTargetPos(kingPos.x + mv.x / 2, kingPos.y);
    king->setPos(kingTargetPos);
    rook->setPos(rookTargetPos);
    boardProgress.registerChange(new PlacementChange(king->getID(), Vector2i(kingPos)));
    updateCastling(king, kingPos);
    boardProgress.registerChange(new PlacementChange(rook->getID(), Vector2i(rookPos)));
    updatePinsDynamic(king->getID(), kingPos, kingTargetPos);
    updatePinsFromKing(pieces[(king->getID() + 16) % 32], rookTargetPos);
    updateEnPassant(rook, false, rookPos, rookTargetPos);
    updateChecks(rook, rookPos);
    updateSideOnMove();
    updateMoveCounts(king, false);
    boardProgress.pushChanges();
    updateObserversByMove(king->getID(), kingPos, kingTargetPos);
    updateObserversByMove(rook->getID(), rookPos, rookTargetPos);
}

int BoardConfig::enPassant(Piece* pawn)
{
    Vector2i mv = getEnPassantVector(pawn);
    Vector2i targetPos = pawn->getPos() + mv;
    int capturedPawnID;
    if (pawn->getColor() == COLOR::WHITE)
    {
        capturedPawnID = table[targetPos.y + 1][targetPos.x]->getID();
        Vector2i oldPos(targetPos.x, targetPos.y + 1);
        removePiece(capturedPawnID);
        updateObserversByMove(capturedPawnID, oldPos, Vector2i(8, 8));
    }
    else
    {
        capturedPawnID = table[targetPos.y - 1][targetPos.x]->getID();
        Vector2i oldPos(targetPos.x, targetPos.y - 1);
        removePiece(capturedPawnID);
        updateObserversByMove(capturedPawnID, oldPos, Vector2i(8, 8));
    }
    movePiece(pawn->getID(), targetPos);
    return capturedPawnID;
}

void BoardConfig::removePiece(int pieceID)
{
    Piece* piece = pieces[pieceID];
    const Vector2i& pos = piece->getPos();
    if (pinVectors[pieceID].x < 8)
    {
        pinVectors[pieceID].x = 8;
        vector<int>& pins = pinnedPieces[(int)piece->getColor()];
        pins.erase(find(pins.begin(), pins.end(), pieceID));
    }
    table[pos.x][pos.y] = nullptr;
    piece->setState(false);
    boardProgress.registerChange(new PlacementChange(piece->getID(), Vector2i(pos)));
}

Piece* BoardConfig::promote(int pawnID, PieceType newType)
{
    Piece* pawn = pieces[pawnID];
    placePiece(pawn->getPos(), pawnID, newType, false);
    boardProgress.registerChange(new PromotionChange(pawnID));
    return pieces[pawnID];
}

bool BoardConfig::undoLastMove()
{
    if (boardProgress.isEmpty())
        return false;
    vector<ConfigChange*>* changelog = boardProgress.getLatestChanges();
    for (int i = changelog->size() - 1; i >= 0; i--)
    {
        ConfigChange* change = changelog->at(i);
        change->applyChange(this);
        delete change;
    }
    delete changelog;
    return true;
}



// Colissions and getters
inline Piece* BoardConfig::checkCollisions(const Vector2i& initPos, const Vector2i& movement, const Vector2i& targetPos)
{
    return nextInDirection(initPos, movement, targetPos, nullptr);
}

Piece* BoardConfig::getKingUnderCheck() const
{
    if (whiteCkecker[0] != nullptr)
        return blackKing;
    if (blackCkecker[0] != nullptr)
        return whiteKing;
    return nullptr;
}




Piece* BoardConfig::isChecked(const sf::Vector2i& square, COLOR color)
{
    King* king;
    if (color == COLOR::WHITE)
        king = blackKing;
    else
        king = whiteKing;
    const dirVector& basicDirs = Queen::directions();
    for (const Vector2i& mv : basicDirs)
    {
        Piece* found = nextInDirection(square, mv, king);
        if (found != nullptr && found->getColor() == color && found->isAttacking(square))
            return found;
    }
    const dirVector& knightDirs = Knight::directions();
    for (const Vector2i& mv : knightDirs)
    {
        Vector2i pos = square + mv;
        if (pos.x >= 0 && pos.x < 8 && pos.y >= 0 && pos.y < 8)
        {
            Piece* found = table[pos.y][pos.x];
            if (found != nullptr && found->getColor() == color && isKnight(found))
                return found;
        }
    }
    return nullptr;
}

bool BoardConfig::isCoveringChecks(Piece* piece, const sf::Vector2i& targetPos)
{
    King* king;
    Piece** checker;
    if (piece->getColor() == COLOR::WHITE) {
        king = whiteKing;
        checker = blackCkecker;
    }
    else
    {
        king = blackKing;
        checker = whiteCkecker;
    }
    if (checker[0] == nullptr || isKing(piece))
        return true;
    else if (checker[1] == nullptr)
    {
        if (targetPos == checker[0]->getPos())
            return true;
        const Vector2i& mv1 = toDirectionalVector2(king->getPos(), checker[0]->getPos());
        const Vector2i& mv2 = toDirectionalVector2(king->getPos(), targetPos);
        if (!isKnight(checker[0]) && mv1 == mv2 && follows(targetPos, mv1, checker[0]->getPos()))
            return true;
    }
    return false;
}

bool BoardConfig::isKingChecked(COLOR side)
{
    if (side == COLOR::WHITE)
       return blackCkecker[0] != nullptr;
    else
        return whiteCkecker[0] != nullptr;
}

// Castling
int BoardConfig::isCastlingAvailable(Piece* king, const sf::Vector2i& mv)
{
    const Vector2i& kingPos = king->getPos();
    int rookID;
    if (mv.x < 0)
    {
        if (king->getColor() == COLOR::WHITE && (!Wlong || blackCkecker[0] != nullptr))
            return 0;
        if (king->getColor() == COLOR::BLACK && (!Blong || whiteCkecker[0] != nullptr))
            return 0;
        if (checkCollisions(kingPos, Vector2i(-1, 0), Vector2i(kingPos.x - 4, kingPos.y)) != nullptr)
            return 0;
        if (isChecked(Vector2i(kingPos.x - 1, kingPos.y), ++king->getColor()))
            return 0;
        if (isChecked(Vector2i(kingPos.x - 2, kingPos.y), ++king->getColor()))
            return 0;
        rookID = table[kingPos.y][kingPos.x - 4]->getID();
    }
    else
    {
        if (king->getColor() == COLOR::WHITE && (!Wshort || blackCkecker[0] != nullptr))
            return 0;
        if (king->getColor() == COLOR::BLACK && (!Bshort || whiteCkecker[0] != nullptr))
            return 0;
        if (checkCollisions(kingPos, Vector2i(1, 0), Vector2i(kingPos.x + 3, kingPos.y)) != nullptr)
            return 0;
        if (isChecked(Vector2i(kingPos.x + 1, kingPos.y), ++king->getColor()))
            return 0;
        if (isChecked(Vector2i(kingPos.x + 2, kingPos.y), ++king->getColor()))
            return 0;
        rookID = table[kingPos.y][kingPos.x + 3]->getID();
    }
    return rookID;
}

int BoardConfig::isCastlingAvailable2(Piece* king, const sf::Vector2i& mv)
{
    const Vector2i& kingPos = king->getPos();
    int rookID;
    if (mv.x < 0)
    {
        if (king->getColor() == COLOR::WHITE && (!Wlong || blackCkecker[0] != nullptr))
            return 0;
        if (king->getColor() == COLOR::BLACK && (!Blong || whiteCkecker[0] != nullptr))
            return 0;
        if (checkCollisions(kingPos, Vector2i(-1, 0), Vector2i(kingPos.x - 4, kingPos.y)) != nullptr)
            return 0;
        if (isChecked2(Vector2i(kingPos.x - 1, kingPos.y), ++king->getColor()))
            return 0;
        if (isChecked2(Vector2i(kingPos.x - 2, kingPos.y), ++king->getColor()))
            return 0;
        rookID = table[kingPos.y][kingPos.x - 4]->getID();
    }
    else
    {
        if (king->getColor() == COLOR::WHITE && (!Wshort || blackCkecker[0] != nullptr))
            return 0;
        if (king->getColor() == COLOR::BLACK && (!Bshort || whiteCkecker[0] != nullptr))
            return 0;
        if (checkCollisions(kingPos, Vector2i(1, 0), Vector2i(kingPos.x + 3, kingPos.y)) != nullptr)
            return 0;
        if (isChecked2(Vector2i(kingPos.x + 1, kingPos.y), ++king->getColor()))
            return 0;
        if (isChecked2(Vector2i(kingPos.x + 2, kingPos.y), ++king->getColor()))
            return 0;
        rookID = table[kingPos.y][kingPos.x + 3]->getID();
    }
    return rookID;
}

// EN PASSANT
bool BoardConfig::isEnPassantPossible(Piece* pawn)
{
    const Vector2i& pawnPos = pawn->getPos();
    if (pawn->getColor() == COLOR::WHITE && pawnPos.y == 3 && (pawnPos.x == enPassantLine - 1 || pawnPos.x == enPassantLine + 1))
        return true;
    if (pawn->getColor() == COLOR::BLACK && pawnPos.y == 4 && (pawnPos.x == enPassantLine - 1 || pawnPos.x == enPassantLine + 1))
        return true;
    return false;
}

Vector2i BoardConfig::getEnPassantVector(Piece* pawn)
{
    if (pawn->getColor() == COLOR::WHITE)
        return Vector2i(enPassantLine - pawn->getPos().x, -1);
    else
        return Vector2i(enPassantLine - pawn->getPos().x, 1);
}

void BoardConfig::showAllPieces() const
{
    for (int i = 0; i < 32; i++)
    {
        if (pieces[i]->isActive())
        {
            std::cout<<"Piece type: "<<(int)pieces[i]->getType()<<std::endl;
            std::cout<<"Piece color: "<<(int)pieces[i]->getColor()<<std::endl;
            std::cout<<"Piece position: ("<<pieces[i]->getPos().x<<","<<pieces[i]->getPos().x<<std::endl;
            std::cout<<std::endl;
        }
    }
}

