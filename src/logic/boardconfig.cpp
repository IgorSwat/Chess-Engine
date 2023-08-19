#include "../engine/movegenerator.h"
#include <exception>
#include <iostream>
#include <algorithm>

using std::vector;
using std::find;

constexpr int BoardConfig::kingID[];
Square BoardConfig::directionalVectors[15][15] {};
vector<Square> BoardConfig::checkSavers[8][8][8][8] {};
vector<Square> BoardConfig::emptySaver {};
bool BoardConfig::directionalComputed = false;
bool BoardConfig::saversComputed = false;




void BoardConfig::initDirectionalVectors()
{
    if (!directionalComputed)
    {
        for (int x = -7; x < 8; x++)
        {
            for (int y = -7; y < 8; y++)
                directionalVectors[7 - x][7 - y] = toDirectionalVector(Square(x, y));
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
                        Square p1(x1, y1);
                        Square p2(x2, y2);
                        if (p1 == p2)
                            checkSavers[x1][y1][x2][y2].push_back(p1);
                        else
                        {
                            Square dir = toDirectionalVector(p1, p2);
                            if (dir.x < 8)
                            {
                                Square currPos = p1;
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

Square BoardConfig::toDirectionalVector(const Square& mv)
{
    if (mv.x == 0 && mv.y == 0)
        return Square(8, 8);
    int x_uns = abs(mv.x);
    int y_uns = abs(mv.y);
    if (mv.x == 0)
        return Square(mv.x, mv.y / y_uns);
    if (mv.y == 0)
        return Square(mv.x / x_uns, mv.y);
    if (x_uns == y_uns)
        return Square(mv.x / x_uns, mv.y / y_uns);
    return Square(8, 8);
}

inline Square BoardConfig::toDirectionalVector(const Square& initPos, const Square& targetPos)
{
    return toDirectionalVector(targetPos - initPos);
}

inline const Square& BoardConfig::toDirectionalVector2(const Square& initPos, const Square& targetPos)
{
    return toDirectionalVector2(targetPos - initPos);
}



void BoardConfig::placePiece(const Square& pos, int pieceID, bool observableFlag)
{
    Piece* piece = pieces[pieceID];
    Square oldPos = pieces[pieceID]->getPos();
    if (table[oldPos.y][oldPos.x] == piece)
        table[oldPos.y][oldPos.x] = nullptr;
    else
        oldPos = Square(8,8);
    table[pos.y][pos.x] = pieces[pieceID];
    pieces[pieceID]->setPos(pos);
    pieces[pieceID]->setState(true);
    updatePiecePlacement(piece->getColor(), piece->getType(), oldPos, pos);
    if (observableFlag)
    {
        updatePinsDynamic(pieceID, oldPos, pos);
        updateObserversByMove(pieceID, oldPos, pos);
    }
}

void BoardConfig::placePiece(const Square& pos, int pieceID, PieceType type, bool observableFlag)
{
    Piece* piece = pieces[pieceID];
    if (piece->getType() != type)
    {
        Side color = piece->getColor();
        Square oldPos = pieces[pieceID]->getPos();
        if (table[oldPos.y][oldPos.x] == piece)
            table[oldPos.y][oldPos.x] = nullptr;
        else
            oldPos = Square(8, 8);
        Square posCopied(pos);
        updatePiecePlacement(color, piece->getType(), oldPos, Square(8, 8));
        delete pieces[pieceID];
        switch (type)
        {
        case PieceType::PAWN:
            pieces[pieceID] = new Pawn(color, pieceID, posCopied);
            break;
        case KNIGHT:
            pieces[pieceID] = new Knight(color, pieceID, posCopied);
            break;
        case BISHOP:
            pieces[pieceID] = new Bishop(color, pieceID, posCopied);
            break;
        case ROOK:
            pieces[pieceID] = new Rook(color, pieceID, posCopied);
            break;
        case QUEEN:
            pieces[pieceID] = new Queen(color, pieceID, posCopied);
            break;
        default:
            throw std::bad_exception();
        }
        table[posCopied.y][posCopied.x] = pieces[pieceID];
        updatePiecePlacement(color, type, Square(8, 8), posCopied);
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
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 5; j++)
            piecePositions[i][j].clear();
    }
    resetPins();
    whiteCkecker[0] = whiteCkecker[1] = nullptr;
    blackCkecker[0] = blackCkecker[1] = nullptr;
    safeMoves[0] = safeMoves[1] = nullptr;
    while (!boardProgress.isEmpty())
        boardProgress.getLatestChanges();
}



Piece* BoardConfig::nextInDirection(const Square& start, const Square& mv, Piece* except)
{
    Square curr = start + mv;
    while (isCorrectSquare(curr))
    {
        Piece* onPath = table[curr.y][curr.x];
        if (onPath != nullptr && onPath != except)
            return onPath;
        curr = curr + mv;
    }
    return nullptr;
}

Piece* BoardConfig::nextInDirection(const Square& start, const Square& mv, const Square& end, Piece* except)
{
    Square curr = start + mv;
    while (curr != end)
    {
        Piece* onPath = table[curr.y][curr.x];
        if (onPath != nullptr && onPath != except)
            return onPath;
        curr = curr + mv;
    }
    return nullptr;
}

bool BoardConfig::follows(const Square& pos, const Square& mv, const Square& target)
{
    Square diffPos(0, 0);
    if (mv.x != 0)
        diffPos.x = (target.x - pos.x) / mv.x;
    if (mv.y != 0)
        diffPos.y = (target.y - pos.y) / mv.y;
    if (diffPos.x > 0 || diffPos.y > 0)
        return true;
    return false;
}




void BoardConfig::searchForPins(Piece* king, const Square& mv)
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

inline void BoardConfig::updatePinsFromKing(Piece* king, const Square& square)
{
    const Square& mv = toDirectionalVector2(king->getPos(), square);
    if (mv.x < 8)
        searchForPins(king, mv);
}

inline void BoardConfig::updatePinsFromSquare(const Square& square)
{
    updatePinsFromKing(whiteKing, square);
    updatePinsFromKing(blackKing, square);
}

void BoardConfig::resetPins()
{
    for (int i = 0; i < 32; i++)
        pinVectors[i] = Square(8,8);
    pinnedPieces[0].clear();
    pinnedPieces[1].clear();
}

Square BoardConfig::isPinned(Piece* piece)
{
    King* king;
    if (piece->getColor() == WHITE)
        king = whiteKing;
    else
        king = blackKing;
    const Square& directionalVector = toDirectionalVector2(king->getPos(), piece->getPos());
    if (directionalVector.x >= 8)
        return directionalVector;
    Piece* found = nextInDirection(king->getPos(), directionalVector, piece);
    if (found == nullptr || found->getColor() == king->getColor())
        return Square(8, 8);
    if (!found->isAttacking(king->getPos()))
        return Square(8, 8);
    return directionalVector;
}



void BoardConfig::updateSideOnMove()
{
    boardProgress.registerChange(new SideOnMoveChange(sideOnMove));
    sideOnMove = opposition(sideOnMove);
}

void BoardConfig::updateMoveCounts(Piece* movedPiece, bool captureFlag)
{
    boardProgress.registerChange(new MoveCountChange(halfMoves, moveNumber));
    if (movedPiece->getColor() == BLACK)
        moveNumber++;
    if (captureFlag || isPawn(movedPiece))
        halfMoves = 0;
    else
        halfMoves++;
}

void BoardConfig::updateEnPassant(Piece* piece, bool captureFlag, const Square& oldPos, const Square& newPos)
{
    int oldEnPassant = enPassantLine;
    int enPassanter1tmp = -1;
    int enPassanter2tmp = -1;
    if (captureFlag || !isPawn(piece) || abs(newPos.y - oldPos.y) != 2)
        enPassantLine = -2;
    else
    {
        enPassantLine = newPos.x;
        Side side = piece->getColor();
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
            const Square& pos = getPiece(enPassanter1)->getPos();
            boardProgress.registerChange(new PlacementChange(enPassanter1, Square(pos)));
            if (generator != nullptr)
                generator->generatePieceMoves(pieces[enPassanter1], getSquaresCoveringChecks(pieces[enPassanter1]->getColor()));
        }
        else if (enPassanter1tmp > 0)
        {
            const Square& pos = getPiece(enPassanter1tmp)->getPos();
            boardProgress.registerChange(new PlacementChange(enPassanter1tmp, Square(pos)));
            if (generator != nullptr)
                generator->generatePieceMoves(pieces[enPassanter1tmp], getSquaresCoveringChecks(pieces[enPassanter1]->getColor()));
        }
        if (enPassanter2 > 0)
        {
            const Square& pos = getPiece(enPassanter2)->getPos();
            boardProgress.registerChange(new PlacementChange(enPassanter2, Square(pos)));
            if (generator != nullptr)
                generator->generatePieceMoves(pieces[enPassanter2], getSquaresCoveringChecks(pieces[enPassanter1]->getColor()));
        }
        else if (enPassanter2tmp > 0)
        {
            const Square& pos = getPiece(enPassanter2tmp)->getPos();
            boardProgress.registerChange(new PlacementChange(enPassanter2tmp, Square(pos)));
            if (generator != nullptr)
                generator->generatePieceMoves(pieces[enPassanter2tmp], getSquaresCoveringChecks(pieces[enPassanter1]->getColor()));
        }
        boardProgress.registerChange(new EnPassantChange(oldEnPassant, enPassanter1, enPassanter2));
        enPassanter1 = enPassanter1tmp;
        enPassanter2 = enPassanter2tmp;
    }
}

void BoardConfig::updateCastling(Piece* piece, const Square& oldPos)
{
    bool Ws {Wshort}, Wl {Wlong}, Bs {Bshort}, Bl {Blong};
    if (piece->getColor() == WHITE)
    {
        if (isKing(piece))
            Wshort = Wlong = false;
        else if (isRook(piece) && oldPos == Square(0, 7))
            Wlong = false;
        else if (isRook(piece) && oldPos == Square(7, 7))
            Wshort = false;
    }
    else
    {
        if (isKing(piece))
            Bshort = Blong = false;
        else if (isRook(piece) && oldPos == Square(0, 0))
            Blong = false;
        else if (isRook(piece) && oldPos == Square(7, 0))
            Bshort = false;
    }
    if (Ws != Wshort || Wl != Wlong || Bs != Bshort || Bl != Blong)
        boardProgress.registerChange(new CastlingChange(Ws, Wl, Bs, Bl));
}

void BoardConfig::updateChecks(Piece* movedPiece, const Square& oldPos)
{
    King* king;
    Piece** checker;
    Piece* oldWhiteCheckers[2] {whiteCkecker[0], whiteCkecker[1]};
    Piece* oldBlackCheckers[2] {blackCkecker[0], blackCkecker[1]};
    vector<Square>* oldSafeMoves[2] {safeMoves[0], safeMoves[1]};
    if (movedPiece->getColor() == WHITE) {
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
    const Square& kingPos = king->getPos();
    const Square& mv = toDirectionalVector2(movedPiece->getPos(), king->getPos());
    if (isKnight(movedPiece) && movedPiece->isReachable(kingPos)) {
        const Square& pos = movedPiece->getPos();
        checker[0] = movedPiece;
        safeMoves[checkedSideID] = &checkSavers[pos.x][pos.y][pos.x][pos.y];
        checkCount += 1;
    }
    else if (mv.x < 8)
    {
        if (movedPiece->isAttacking(kingPos) && nextInDirection(movedPiece->getPos(), mv, nullptr) == king)
        {
            const Square& pos = movedPiece->getPos();
            checker[0] = movedPiece;
            safeMoves[checkedSideID] = &checkSavers[pos.x][pos.y][kingPos.x - mv.x][kingPos.y - mv.y];
            checkCount += 1;
        }
    }
    const Square& mv1 = toDirectionalVector2(kingPos, oldPos);
    const Square& mv2 = toDirectionalVector2(kingPos, movedPiece->getPos());
    if (mv1.x < 8 && mv1 != mv2)
    {
        Piece* found = nextInDirection(kingPos, mv1, movedPiece);
        if (found != nullptr && found->getColor() != king->getColor() && found->isAttacking(kingPos))
        {
            const Square& pos = found->getPos();
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

void BoardConfig::updatePinsDynamic(int pieceID, const Square& oldPos, const Square& newPos)
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
        for (const Square& dir : dirs)
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

void BoardConfig::updatePiecePlacement(Side side, PieceType type, const Square& oldPos, const Square& newPos)
{
    if (type != PAWN && type != KING)
    {
        vector<Square>& positions = piecePositions[side][type];
        if (oldPos.x < 8 && newPos.x < 8)
            *(std::find(positions.begin(), positions.end(), oldPos)) = newPos;
        else if (oldPos.x < 8)
            positions.erase(std::find(positions.begin(), positions.end(), oldPos));
        else if (newPos.x < 8)
            positions.push_back(newPos);
    }
}

void BoardConfig::updatePinsStatic()
{
    for (int i = 0; i < 32; i++)
    {
        Piece* piece = pieces[i];
        Square pinDir;
        if (piece->isActive() && (pinDir = isPinned(piece)).x < 8)
        {
            pinVectors[i] = pinDir;
            pinnedPieces[i / 16].push_back(i);
        }
    }
}

void BoardConfig::updateChecksForSide(Side side)
{

    Piece* king;
    Piece** checker;
    int checkedSideID;
    if (side == WHITE)
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
    for (const Square& dir : dirs)
    {
        Piece* firstFind = nextInDirection(king->getPos(), dir, nullptr);
        if (firstFind != nullptr && firstFind->getColor() != side && firstFind->isAttacking(king->getPos()))
        {
            const Square& pos = firstFind->getPos();
            checker[checkCount] = firstFind;
            checkCount += 1;
            if (checkCount == 1)
                safeMoves[checkedSideID] = &checkSavers[pos.x][pos.y][king->getPos().x + dir.x][king->getPos().y + dir.y];
            else
                safeMoves[checkedSideID] = &emptySaver;
        }
    }
    const dirVector& knightDirs = Knight::directions();
    for (const Square& dir : knightDirs)
    {
        Square pos = king->getPos() + dir;
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
    updateChecksForSide(WHITE);
    updateChecksForSide(BLACK);
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

    whiteKing = new King(WHITE, 0, Square(4, 7));
    pieces[0] = whiteKing;
    for (int i = 0; i < 8; i++)
        pieces[1 + i] = new Pawn(WHITE, 1 + i, Square(i, 6));
    pieces[9] = new Knight(WHITE, 9, Square(1, 7));
    pieces[10] = new Knight(WHITE, 10, Square(6, 7));
    pieces[11] = new Bishop(WHITE, 11, Square(2, 7));
    pieces[12] = new Bishop(WHITE, 12, Square(5, 7));
    pieces[13] = new Rook(WHITE, 13, Square(0, 7));
    pieces[14] = new Rook(WHITE, 14, Square(7, 7));
    pieces[15] = new Queen(WHITE, 15, Square(3, 7));

    blackKing = new King(BLACK, blackID, Square(4, 0));
    pieces[blackID] = blackKing;
    for (int i = 0; i < 8; i++)
        pieces[blackID + 1 + i] = new Pawn(BLACK, blackID + 1 + i, Square(i, 1));
    pieces[blackID + 9] = new Knight(BLACK, blackID + 9, Square(1, 0));
    pieces[blackID + 10] = new Knight(BLACK, blackID + 10, Square(6, 0));
    pieces[blackID + 11] = new Bishop(BLACK, blackID + 11, Square(2, 0));
    pieces[blackID + 12] = new Bishop(BLACK, blackID + 12, Square(5, 0));
    pieces[blackID + 13] = new Rook(BLACK, blackID + 13, Square(0, 0));
    pieces[blackID + 14] = new Rook(BLACK, blackID + 14, Square(7, 0));
    pieces[blackID + 15] = new Queen(BLACK, blackID + 15, Square(3, 0));

    setToDefault();

}

BoardConfig::BoardConfig(const BoardConfig& config)
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            table[i][j] = nullptr;
            attacksTable[i][j][0] = config.attacksTable[i][j][0];
            attacksTable[i][j][1] = config.attacksTable[i][j][1];
        }
    }

    for (int i = 0; i < 32; i++)
    {
        Piece* piece = config.pieces[i];
        switch (piece->getType())
        {
        case PAWN:
            pieces[i] = new Pawn(*dynamic_cast<Pawn*>(piece));
            break;
        case KNIGHT:
            pieces[i] = new Knight(*dynamic_cast<Knight*>(piece));
            break;
        case BISHOP:
            pieces[i] = new Bishop(*dynamic_cast<Bishop*>(piece));
            break;
        case ROOK:
            pieces[i] = new Rook(*dynamic_cast<Rook*>(piece));
            break;
        case QUEEN:
            pieces[i] = new Queen(*dynamic_cast<Queen*>(piece));
            break;
        case KING:
            pieces[i] = new King(*dynamic_cast<King*>(piece));
            break;
        }
    }
    whiteKing = dynamic_cast<King*>(pieces[0]);
    blackKing = dynamic_cast<King*>(pieces[16]);

    for (int i = 0; i < 32; i++)
    {
        if (pieces[i]->isActive())
            table[pieces[i]->getPos().y][pieces[i]->getPos().x] = pieces[i];
    }

    sideOnMove = config.sideOnMove;
    Wshort = config.Wshort;
    Wlong = config.Wlong;
    Bshort = config.Bshort;
    Blong = config.Blong;
    enPassantLine = config.enPassantLine;
    enPassanter1 = config.enPassanter1;
    enPassanter2 = config.enPassanter2;
    moveNumber = config.moveNumber;
    halfMoves = config.halfMoves;

    updatePinsStatic();
    updateChecksStatic();
}

BoardConfig& BoardConfig::operator=(const BoardConfig& config)
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            table[i][j] = nullptr;
            attacksTable[i][j][0] = config.attacksTable[i][j][0];
            attacksTable[i][j][1] = config.attacksTable[i][j][1];
        }
    }

    for (int i = 0; i < 32; i++)
    {
        Piece* piece = config.pieces[i];
        pieces[i]->setState(piece->isActive());
        if (pieces[i]->isActive())
        {
            placePiece(piece->getPos(), i, piece->getType(), false);
            table[pieces[i]->getPos().y][pieces[i]->getPos().x] = pieces[i];
        }
    }

    sideOnMove = config.sideOnMove;
    Wshort = config.Wshort;
    Wlong = config.Wlong;
    Bshort = config.Bshort;
    Blong = config.Blong;
    enPassantLine = config.enPassantLine;
    enPassanter1 = config.enPassanter1;
    enPassanter2 = config.enPassanter2;
    moveNumber = config.moveNumber;
    halfMoves = config.halfMoves;

    updatePinsStatic();
    updateChecksStatic();
    return *this;
}

BoardConfig::~BoardConfig()
{
    for (int i = 0; i < 32; i++)
        delete pieces[i];
}

void BoardConfig::setToDefault()
{
    sideOnMove = WHITE;
    Wshort = Wlong = Bshort = Blong = true;
    enPassantLine = -2;
    moveNumber = 1;
    halfMoves = 0;

    clearBoard();

    placePiece(Square(4, 7), 0, false);
    for (int i = 0; i < 8; i++)
        placePiece(Square(i, 6), 1 + i, PieceType::PAWN, false);
    placePiece(Square(1, 7), 9, KNIGHT, false);
    placePiece(Square(6, 7), 10, KNIGHT, false);
    placePiece(Square(2, 7), 11, BISHOP, false);
    placePiece(Square(5, 7), 12, BISHOP, false);
    placePiece(Square(0, 7), 13, ROOK, false);
    placePiece(Square(7, 7), 14, ROOK, false);
    placePiece(Square(3, 7), 15, QUEEN, false);

    placePiece(Square(4, 0), blackID, false);
    for (int i = 0; i < 8; i++)
        placePiece(Square(i, 1), blackID + 1 + i, PieceType::PAWN, false);
    placePiece(Square(1, 0), blackID + 9, KNIGHT, false);
    placePiece(Square(6, 0), blackID + 10, KNIGHT, false);
    placePiece(Square(2, 0), blackID + 11, BISHOP, false);
    placePiece(Square(5, 0), blackID + 12, BISHOP, false);
    placePiece(Square(0, 0), blackID + 13, ROOK, false);
    placePiece(Square(7, 0), blackID + 14, ROOK, false);
    placePiece(Square(3, 0), blackID + 15, QUEEN, false);

    updatePinsStatic();
    updateEngines();
}

bool BoardConfig::setFromFEN(const std::string& FEN)
{
    this->FENreader.setFEN(FEN);
    clearBoard();
    if (this->FENreader.parseToConfig(this))
    {
        updatePinsStatic();
        updateChecksStatic();
        updateEngines();
        return true;
    }
    return false;
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



// Moving and removing pieces
void BoardConfig::movePiece(int pieceID, const Square& newPos, PieceType promoteTo)
{
    bool captureFlag = false;
    Piece* piece = pieces[pieceID];
    Square oldPos(piece->getPos());
    PieceType type = piece->getType();
    if (table[newPos.y][newPos.x] != nullptr)
    {
        Piece* removed = table[newPos.y][newPos.x];
        int id = removed->getID();
        boardProgress.registerChange(new PlacementChange(id, Square(newPos)));
        table[newPos.y][newPos.x]->setState(false);
        captureFlag = true;
        if (pinVectors[id].x < 8)
        {
            pinVectors[id].x = 8;
            vector<int>& pins = pinnedPieces[removed->getColor()];
            pins.erase(find(pins.begin(), pins.end(), id));

        }
        updatePiecePlacement(removed->getColor(), removed->getType(), newPos, Square(8, 8));
        updateObserversByMove(id, newPos, Square(9, 8));
    }
    table[newPos.y][newPos.x] = piece;
    table[oldPos.y][oldPos.x] = nullptr;
    piece->setPos(newPos);
    updatePiecePlacement(piece->getColor(), piece->getType(), oldPos, newPos);
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
    updateEngines();
}

void BoardConfig::castle(Piece* king, const Square& mv)
{
    Square kingPos(king->getPos());
    Piece* rook;
    if (mv.x < 0)
        rook = table[kingPos.y][kingPos.x - 4];
    else
        rook = table[kingPos.y][kingPos.x + 3];
    Square rookPos(rook->getPos());
    table[kingPos.y][kingPos.x + mv.x] = king;
    table[kingPos.y][kingPos.x] = nullptr;
    table[rookPos.y][rookPos.x] = nullptr;
    table[kingPos.y][kingPos.x + mv.x / 2] = rook;
    Square kingTargetPos = kingPos + mv;
    Square rookTargetPos(kingPos.x + mv.x / 2, kingPos.y);
    king->setPos(kingTargetPos);
    rook->setPos(rookTargetPos);
    boardProgress.registerChange(new PlacementChange(king->getID(), Square(kingPos)));
    updateCastling(king, kingPos);
    boardProgress.registerChange(new PlacementChange(rook->getID(), Square(rookPos)));
    updatePinsDynamic(king->getID(), kingPos, kingTargetPos);
    updatePinsFromKing(pieces[(king->getID() + 16) % 32], rookTargetPos);
    updateEnPassant(rook, false, rookPos, rookTargetPos);
    updateChecks(rook, rookPos);
    updateSideOnMove();
    updateMoveCounts(king, false);
    boardProgress.pushChanges();
    updateObserversByMove(king->getID(), kingPos, kingTargetPos);
    updateObserversByMove(rook->getID(), rookPos, rookTargetPos);
    updatePiecePlacement(king->getColor(), ROOK, rookPos, rookTargetPos);
    updateEngines();
}

int BoardConfig::enPassant(Piece* pawn)
{
    Square mv = getEnPassantVector(pawn);
    Square targetPos = pawn->getPos() + mv;
    int capturedPawnID;
    if (pawn->getColor() == WHITE)
    {
        capturedPawnID = table[targetPos.y + 1][targetPos.x]->getID();
        Square oldPos(targetPos.x, targetPos.y + 1);
        removePiece(capturedPawnID);
        updateObserversByMove(capturedPawnID, oldPos, Square(8, 8));
    }
    else
    {
        capturedPawnID = table[targetPos.y - 1][targetPos.x]->getID();
        Square oldPos(targetPos.x, targetPos.y - 1);
        removePiece(capturedPawnID);
        updateObserversByMove(capturedPawnID, oldPos, Square(8, 8));
    }
    movePiece(pawn->getID(), targetPos);
    return capturedPawnID;
}

void BoardConfig::removePiece(int pieceID)
{
    Piece* piece = pieces[pieceID];
    const Square& pos = piece->getPos();
    if (pinVectors[pieceID].x < 8)
    {
        pinVectors[pieceID].x = 8;
        vector<int>& pins = pinnedPieces[(int)piece->getColor()];
        pins.erase(find(pins.begin(), pins.end(), pieceID));
    }
    table[pos.x][pos.y] = nullptr;
    piece->setState(false);
    updatePiecePlacement(piece->getColor(), piece->getType(), pos, Square(8, 8));
    boardProgress.registerChange(new PlacementChange(piece->getID(), Square(pos)));
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
    updateEngines();
    return true;
}



// Colissions and getters
Piece* BoardConfig::getKingUnderCheck() const
{
    if (whiteCkecker[0] != nullptr)
        return blackKing;
    if (blackCkecker[0] != nullptr)
        return whiteKing;
    return nullptr;
}




Piece* BoardConfig::isChecked(const Square& square, Side color)
{
    King* king;
    if (color == WHITE)
        king = blackKing;
    else
        king = whiteKing;
    const dirVector& basicDirs = Queen::directions();
    for (const Square& mv : basicDirs)
    {
        Piece* found = nextInDirection(square, mv, king);
        if (found != nullptr && found->getColor() == color && found->isAttacking(square))
            return found;
    }
    const dirVector& knightDirs = Knight::directions();
    for (const Square& mv : knightDirs)
    {
        Square pos = square + mv;
        if (pos.x >= 0 && pos.x < 8 && pos.y >= 0 && pos.y < 8)
        {
            Piece* found = table[pos.y][pos.x];
            if (found != nullptr && found->getColor() == color && isKnight(found))
                return found;
        }
    }
    return nullptr;
}

bool BoardConfig::isCoveringChecks(Piece* piece, const Square& targetPos)
{
    King* king;
    Piece** checker;
    if (piece->getColor() == WHITE) {
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
        const Square& mv1 = toDirectionalVector2(king->getPos(), checker[0]->getPos());
        const Square& mv2 = toDirectionalVector2(king->getPos(), targetPos);
        if (!isKnight(checker[0]) && mv1 == mv2 && follows(targetPos, mv1, checker[0]->getPos()))
            return true;
    }
    return false;
}

bool BoardConfig::isKingChecked(Side side)
{
    if (side == WHITE)
       return blackCkecker[0] != nullptr;
    else
        return whiteCkecker[0] != nullptr;
}

// Castling
int BoardConfig::isCastlingAvailable(Piece* king, const Square& mv)
{
    const Square& kingPos = king->getPos();
    int rookID;
    if (mv.x < 0)
    {
        if (king->getColor() == WHITE && (!Wlong || blackCkecker[0] != nullptr))
            return 0;
        if (king->getColor() == BLACK && (!Blong || whiteCkecker[0] != nullptr))
            return 0;
        if (checkCollisions(kingPos, Square(-1, 0), Square(kingPos.x - 4, kingPos.y)) != nullptr)
            return 0;
        if (isChecked(Square(kingPos.x - 1, kingPos.y), opposition(king->getColor())))
            return 0;
        if (isChecked(Square(kingPos.x - 2, kingPos.y), opposition(king->getColor())))
            return 0;
        rookID = table[kingPos.y][kingPos.x - 4]->getID();
    }
    else
    {
        if (king->getColor() == WHITE && (!Wshort || blackCkecker[0] != nullptr))
            return 0;
        if (king->getColor() == BLACK && (!Bshort || whiteCkecker[0] != nullptr))
            return 0;
        if (checkCollisions(kingPos, Square(1, 0), Square(kingPos.x + 3, kingPos.y)) != nullptr)
            return 0;
        if (isChecked(Square(kingPos.x + 1, kingPos.y), opposition(king->getColor())))
            return 0;
        if (isChecked(Square(kingPos.x + 2, kingPos.y), opposition(king->getColor())))
            return 0;
        rookID = table[kingPos.y][kingPos.x + 3]->getID();
    }
    return rookID;
}

int BoardConfig::isCastlingAvailable2(Piece* king, const Square& mv)
{
    const Square& kingPos = king->getPos();
    int rookID;
    if (mv.x < 0)
    {
        if (king->getColor() == WHITE && (!Wlong || blackCkecker[0] != nullptr))
            return 0;
        if (king->getColor() == BLACK && (!Blong || whiteCkecker[0] != nullptr))
            return 0;
        if (checkCollisions(kingPos, Square(-1, 0), Square(kingPos.x - 4, kingPos.y)) != nullptr)
            return 0;
        if (isChecked2(Square(kingPos.x - 1, kingPos.y), opposition(king->getColor())))
            return 0;
        if (isChecked2(Square(kingPos.x - 2, kingPos.y), opposition(king->getColor())))
            return 0;
        rookID = table[kingPos.y][kingPos.x - 4]->getID();
    }
    else
    {
        if (king->getColor() == WHITE && (!Wshort || blackCkecker[0] != nullptr))
            return 0;
        if (king->getColor() == BLACK && (!Bshort || whiteCkecker[0] != nullptr))
            return 0;
        if (checkCollisions(kingPos, Square(1, 0), Square(kingPos.x + 3, kingPos.y)) != nullptr)
            return 0;
        if (isChecked2(Square(kingPos.x + 1, kingPos.y), opposition(king->getColor())))
            return 0;
        if (isChecked2(Square(kingPos.x + 2, kingPos.y), opposition(king->getColor())))
            return 0;
        rookID = table[kingPos.y][kingPos.x + 3]->getID();
    }
    return rookID;
}

// EN PASSANT
bool BoardConfig::isEnPassantPossible(Piece* pawn)
{
    const Square& pawnPos = pawn->getPos();
    if (pawn->getColor() == WHITE && pawnPos.y == 3 && (pawnPos.x == enPassantLine - 1 || pawnPos.x == enPassantLine + 1))
        return true;
    if (pawn->getColor() == BLACK && pawnPos.y == 4 && (pawnPos.x == enPassantLine - 1 || pawnPos.x == enPassantLine + 1))
        return true;
    return false;
}

Square BoardConfig::getEnPassantVector(Piece* pawn)
{
    if (pawn->getColor() == WHITE)
        return Square(enPassantLine - pawn->getPos().x, -1);
    else
        return Square(enPassantLine - pawn->getPos().x, 1);
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

