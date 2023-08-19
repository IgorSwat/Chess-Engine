#include "movegenerator.h"
#include <algorithm>
using std::vector;
using std::unordered_set;
using std::find;
using std::cout;
using std::endl;

/// Helper functions
void MoveGenerator::initializeMoveLists()
{
    for (int i = 0; i < 32; i++)
        configureMoveList(i);

}

void MoveGenerator::configureMoveList(int pieceID)
{
    PieceType type = config->getPiece(pieceID)->getType();
    switch (type)
    {
    case PAWN:
        legalMoves[pieceID].reset(3);
        attacksOnly[pieceID].reset(3);
        if (pieceID < 16)
        {
            legalMoves[pieceID].setKeys({ Square(-1, -1), Square(0, -1), Square(1, -1) });
            attacksOnly[pieceID].setKeys({ Square(-1, -1), Square(0, -1), Square(1, -1) });
        }
        else
        {
            legalMoves[pieceID].setKeys({ Square(-1, 1), Square(0, 1), Square(1, 1) });
            attacksOnly[pieceID].setKeys({ Square(-1, 1), Square(0, 1), Square(1, 1) });
        }
        break;
    case KNIGHT:
        setListKeys(pieceID, 8, Knight::directions());
        break;
    case BISHOP:
        setListKeys(pieceID, 4, Bishop::directions());
        break;
    case ROOK:
        setListKeys(pieceID, 4, Rook::directions());
        break;
    case QUEEN:
        setListKeys(pieceID, 8, Queen::directions());
        break;
    case KING:
        legalMoves[pieceID].reset(1);
        attacksOnly[pieceID].reset(1);
        break;
    }
}

void MoveGenerator::setListKeys(int pieceID, int directionCount, const dirVector& directions)
{
    legalMoves[pieceID].reset(directionCount);
    attacksOnly[pieceID].reset(directionCount);
    for (const Square& dir : directions)
    {
        legalMoves[pieceID].setKey(dir);
        attacksOnly[pieceID].setKey(dir);
    }
}

inline void MoveGenerator::addLegalMove(const Square& dir, int pieceID, PieceType type, const Square& targetPos, const MoveMask& mask)
{
    Move2 move(pieceID, type, targetPos, mask);
    legalMoves[pieceID].add(dir, move);
    updateObserversByInsertion(move);
    squareDependable[targetPos.y][targetPos.x].push_back(pieceID);
}

inline void MoveGenerator::addPseudoLegalMove(const Square& dir, int pieceID, PieceType type, const Square& targetPos, const MoveMask& mask)
{
    Move2 move(pieceID, type, targetPos, mask);
    attacksOnly[pieceID].add(dir, move);
    updateObserversByInsertion(move);
    squareDependable[targetPos.y][targetPos.x].push_back(pieceID);
}

void MoveGenerator::removeMoves(int pieceID, const Square& dir)
{
    const vector<Move2>& legal = legalMoves[pieceID].getMoves(dir);
    const vector<Move2>& attacks = attacksOnly[pieceID].getMoves(dir);
    updateObserversByRemoval(pieceID, legal, true);
    updateObserversByRemoval(pieceID, attacks, false);
    for (const Move2& move : legal)
    {
        vector<int>& square = squareDependable[move.targetPos.y][move.targetPos.x];
        vector<int>::iterator it = find(square.begin(), square.end(), pieceID);
        if (it != square.end())
            square.erase(it);
    }
    for (const Move2& move : attacks)
    {
        vector<int>& square = squareDependable[move.targetPos.y][move.targetPos.x];
        vector<int>::iterator it = find(square.begin(), square.end(), pieceID);
        if (it != square.end())
            square.erase(it);
    }
    legalMoves[pieceID].erase(dir);
    attacksOnly[pieceID].erase(dir);
}

void MoveGenerator::removeMoves(int pieceID)
{
    updateObserversByRemoval(pieceID, legalMoves[pieceID], true);
    updateObserversByRemoval(pieceID, attacksOnly[pieceID], false);
    for (const Move2& move : legalMoves[pieceID])
    {
        vector<int>& square = squareDependable[move.targetPos.y][move.targetPos.x];
        vector<int>::iterator it = find(square.begin(), square.end(), pieceID);
        if (it != square.end())
            square.erase(it);
    }
    for (const Move2& move : attacksOnly[pieceID])
    {
        vector<int>& square = squareDependable[move.targetPos.y][move.targetPos.x];
        vector<int>::iterator it = find(square.begin(), square.end(), pieceID);
        if (it != square.end())
            square.erase(it);
    }
    legalMoves[pieceID].clear();
    attacksOnly[pieceID].clear();
}



/// Specyfic piece type generators
void MoveGenerator::generatePawnMoves(Piece* pawn, vector<Square>* checkSavers)
{
    int pawnID = pawn->getID();
    const Square& currPos = pawn->getPos();
    const Square& pinVector = config->isPinned2(pawn);
    bool pinFlag = pinVector.x < 8 ? true : false;
    if (pawn->getColor() == WHITE)
    {
        if (currPos.x > 0)
            generatePawnMoves(Square(-1, -1), pawn, checkSavers);
        generatePawnMoves(Square(0, -1), pawn, checkSavers);
        if (currPos.x < 7)
            generatePawnMoves(Square(1, -1), pawn, checkSavers);
        if (config->isEnPassantPossible(pawn))
        {
            Square movementVector(config->getEnPassantLine() - currPos.x, -1);
            Square targetPos = currPos + movementVector;
            if ((!pinFlag || pinVector == movementVector || pinVector == -movementVector) &&
                (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
                addLegalMove(movementVector, pawnID, PAWN, targetPos, MoveType::COMMON | MoveType::ENPASSANT);
        }
    }
    else
    {
        if (currPos.x > 0)
            generatePawnMoves(Square(-1, 1), pawn, checkSavers);
        generatePawnMoves(Square(0, 1), pawn, checkSavers);
        if (currPos.x < 7)
            generatePawnMoves(Square(1, 1), pawn, checkSavers);
        if (config->isEnPassantPossible(pawn))
        {
            Square movementVector(config->getEnPassantLine() - currPos.x, 1);
            Square targetPos = currPos + movementVector;
            if ((!pinFlag || pinVector == movementVector || pinVector == -movementVector) &&
                (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
                addLegalMove(movementVector, pawnID, PAWN, targetPos, MoveType::COMMON | MoveType::ENPASSANT);
        }
    }
}

void MoveGenerator::generatePawnMoves(const Square& dir, Piece* pawn, vector<Square>* checkSavers)
{
    int pawnID = pawn->getID();
    const Square& pinVector = config->isPinned2(pawn);
    bool pinFlag = pinVector.x < 8 ? true : false;
    if (dir.x == 0)
    {
        bool pinCheck = !pinFlag || pinVector == dir || pinVector == -dir;
        Square targetPos = pawn->getPos() + dir;
        if (config->getPiece(targetPos) == nullptr && pinCheck &&
            (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
        {
            if (targetPos.y == 0)
            {
                Move2 move1(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON, QUEEN);
                legalMoves[pawnID].add(dir, move1);
                updateObserversByInsertion(move1);
                Move2 move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON, KNIGHT);
                legalMoves[pawnID].add(dir, move2);
                updateObserversByInsertion(move2);
                Move2 move3(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON, ROOK);
                legalMoves[pawnID].add(dir, move3);
                updateObserversByInsertion(move3);
                Move2 move4(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON, BISHOP);
                legalMoves[pawnID].add(dir, move4);
                updateObserversByInsertion(move4);
                squareDependable[targetPos.y][targetPos.x].push_back(pawnID);
            }
            else
                addLegalMove(dir, pawnID, PAWN, targetPos, MoveType::COMMON);
        }
        else
            addPseudoLegalMove(dir, pawnID, PAWN, targetPos, MoveType::NONE);
        if (pawn->getPos().y == 6 && pawn->getColor() == WHITE)
        {
            targetPos = pawn->getPos() + Square(0, -2);
            if (config->getPiece(Square(pawn->getPos().x, pawn->getPos().y - 1)) == nullptr &&
                config->getPiece(targetPos) == nullptr && pinCheck &&
                (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
                addLegalMove(dir, pawnID, PAWN, targetPos, MoveType::COMMON);
            else
                addPseudoLegalMove(dir, pawnID, PAWN, targetPos, MoveType::NONE);
        }
        else if (pawn->getPos().y == 1 && pawn->getColor() == BLACK)
        {
            targetPos = pawn->getPos() + Square(0, 2);
            if (config->getPiece(Square(pawn->getPos().x, pawn->getPos().y + 1)) == nullptr &&
                config->getPiece(targetPos) == nullptr && pinCheck &&
                (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
                addLegalMove(dir, pawnID, PAWN, targetPos, MoveType::COMMON);
            else
                addPseudoLegalMove(dir, pawnID, PAWN, targetPos, MoveType::NONE);
        }
    }
    else if (!pinFlag || pinVector == dir || pinVector == -dir)
    {
        Square targetPos = pawn->getPos() + dir;
        if (config->getPiece(targetPos) != nullptr && config->getPiece(targetPos)->getColor() != pawn->getColor() &&
            (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
        {
            if (targetPos.y == 0 || targetPos.y == 7)
            {
                Move2 move1(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK, QUEEN);
                legalMoves[pawnID].add(dir, move1);
                updateObserversByInsertion(move1);
                Move2 move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK, KNIGHT);
                legalMoves[pawnID].add(dir, move2);
                updateObserversByInsertion(move2);
                Move2 move3(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK, ROOK);
                legalMoves[pawnID].add(dir, move3);
                updateObserversByInsertion(move3);
                Move2 move4(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK, BISHOP);
                legalMoves[pawnID].add(dir, move4);
                updateObserversByInsertion(move4);
                squareDependable[targetPos.y][targetPos.x].push_back(pawnID);
            }
            else
                addLegalMove(dir, pawnID, PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK);
        }
        else
            addPseudoLegalMove(dir, pawnID, PAWN, targetPos, MoveType::ATTACK);
    }
}

void MoveGenerator::generateKnightMoves(Piece* knight, vector<Square>* checkSavers)
{
    const Square& pinVector = config->isPinned2(knight);
    if (pinVector.x < 8)
        return;
    int knightID = knight->getID();
    const Square& currPos = knight->getPos();
    const dirVector& directions = knight->getDirections();
    for (const Square& dir : directions)
    {
        Square targetPos = currPos + dir;
        Piece* piece = config->getPiece(targetPos);
        if (targetPos.x >= 0 && targetPos.y >= 0 && targetPos.y < 8 && targetPos.x < 8)
        {
            if ((checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()) &&
                (piece == nullptr || piece->getColor() != knight->getColor()))
                addLegalMove(dir, knightID, KNIGHT, targetPos, MoveType::COMMON | MoveType::ATTACK);
            else
                addPseudoLegalMove(dir, knightID, KNIGHT, targetPos, MoveType::ATTACK);
        }
    }
}

void MoveGenerator::generateKnightMoves(const Square& dir, Piece* knight, vector<Square>* checkSavers)
{
    Square targetPos = knight->getPos() + dir;
    Piece* piece = config->getPiece(targetPos);
    if ((checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()) &&
        (piece == nullptr || piece->getColor() != knight->getColor()))
        addLegalMove(dir, knight->getID(), KNIGHT, targetPos, MoveType::COMMON | MoveType::ATTACK);
    else
        addPseudoLegalMove(dir, knight->getID(), KNIGHT, targetPos, MoveType::ATTACK);
}

void MoveGenerator::generateKingMoves(Piece* king, vector<Square>* checkSavers)
{
    Side side = king->getColor();
    int kingID = king->getID();
    const Square& currPos = king->getPos();
    const dirVector& directions = king->getDirections();
    for (const Square& dir : directions)
    {
        Square targetPos = currPos + dir;
        if (targetPos.x >= 0 && targetPos.x < 8 && targetPos.y >= 0 && targetPos.y < 8)
        {
            Piece* piece = config->getPiece(targetPos);
            if (!config->isChecked2(targetPos, opposition(side)) && (piece == nullptr || piece->getColor() != king->getColor()))
            {
                Move2 move(kingID, PieceType::KING, targetPos, MoveType::COMMON | MoveType::ATTACK);
                legalMoves[kingID].add(dir, move);
                updateObserversByInsertion(move);
            }
            else
            {
                Move2 move(kingID, PieceType::KING, targetPos, MoveType::ATTACK);
                attacksOnly[kingID].add(dir, move);
                updateObserversByInsertion(move);
            }
        }
    }
    if (currPos.x > 1 && config->isCastlingAvailable2(king, Square(-2, 0)) > 0)
    {
        Move2 move(kingID, PieceType::KING, Square(currPos.x - 2, currPos.y), MoveType::COMMON | MoveType::CASTLE);
        legalMoves[kingID].add(Square(-1, 0), move);
        updateObserversByInsertion(move);
    }
    if (currPos.x < 6 && config->isCastlingAvailable2(king, Square(2, 0)) > 0)
    {
        Move2 move(kingID, PieceType::KING, Square(currPos.x + 2, currPos.y), MoveType::COMMON | MoveType::CASTLE);
        legalMoves[kingID].add(Square(1, 0), move);
        updateObserversByInsertion(move);
    }
}

inline void MoveGenerator::registerMove(const Square& dir, int pieceID, PieceType type, const Square& targetPos,
    vector<Square>* checkSavers, bool batteryFlag)
{
    if (!batteryFlag && (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
        addLegalMove(dir, pieceID, type, targetPos, MoveType::COMMON | MoveType::ATTACK);
    else
        addPseudoLegalMove(dir, pieceID, type, targetPos, MoveType::ATTACK);
}

void MoveGenerator::generateOthersMoves(Piece* other, vector<Square>* checkSavers)
{
    const Square& pinVector = config->isPinned2(other);
    Side side = other->getColor();
    int pieceID = other->getID();
    const Square& currPos = other->getPos();
    const dirVector& directions = other->getDirections();
    int oppositeSideID = (int)opposition(side) * 16;
    for (const Square& dir : directions)
    {
        bool isRookDir = dir.x == 0 || dir.y == 0;
        PieceType type = other->getType();
        if (pinVector.x < 8 && dir != pinVector && dir != -pinVector)
            continue;
        Square targetPos = currPos + dir;
        bool batteryFlag = false;
        Piece* foundPiece;
        while (isCorrectSquare(targetPos))
        {
            foundPiece = config->getPiece(targetPos);
            if (foundPiece != nullptr)
            {
                if (foundPiece->getColor() != side)
                {
                    registerMove(dir, pieceID, type, targetPos, checkSavers, batteryFlag);
                    if (foundPiece->getID() == oppositeSideID)
                        batteryFlag = true;
                    else
                        break;
                }
                else
                {
                    addPseudoLegalMove(dir, pieceID, type, targetPos, MoveType::ATTACK);
                    if ((isRookDir && foundPiece->hasRookAbilities()) || (!isRookDir && foundPiece->hasBishopAbilities()))
                    {
                        batteryFlag = true;
                        if (foundPiece->getID() != oppositeSideID && (int) type < (int) foundPiece->getType())
                            type = foundPiece->getType();
                        targetPos = targetPos + dir;
                        continue;
                    }
                    else
                        break;
                }
            }
            registerMove(dir, pieceID, type, targetPos, checkSavers, batteryFlag);
            targetPos = targetPos + dir;
        }
    }
}

void MoveGenerator::generateOthersMoves(const Square& dir, Piece* other, vector<Square>* checkSavers)
{
    Side side = other->getColor();
    int oppositeSideID = (int)opposition(side) * 16;
    int pieceID = other->getID();
    bool isRookDir = dir.x == 0 || dir.y == 0;
    PieceType type = other->getType();
    Square targetPos = other->getPos() + dir;
    bool batteryFlag = false;
    Piece* foundPiece;
    while (isCorrectSquare(targetPos))
    {
        foundPiece = config->getPiece(targetPos);
        if (foundPiece != nullptr)
        {
            if (foundPiece->getColor() != side)
            {
                registerMove(dir, pieceID, type, targetPos, checkSavers, batteryFlag);
                if (foundPiece->getID() == oppositeSideID)
                    batteryFlag = true;
                else
                    break;
            }
            else
            {
                addPseudoLegalMove(dir, pieceID, type, targetPos, MoveType::ATTACK);
                if ((isRookDir && foundPiece->hasRookAbilities()) || (!isRookDir && foundPiece->hasBishopAbilities()))
                {
                    batteryFlag = true;
                    if (foundPiece->getID() != oppositeSideID && (int)type < (int)foundPiece->getType())
                        type = foundPiece->getType();
                    targetPos = targetPos + dir;
                    continue;
                }
                else
                    break;
            }
        }
        registerMove(dir, pieceID, type, targetPos, checkSavers, batteryFlag);
        targetPos = targetPos + dir;
    }
}



/// Generic move generators
void MoveGenerator::generatePieceMoves(Piece* piece, vector<Square>* checkSavers)
{
    removeMoves(piece->getID());
    switch (piece->getType())
    {
    case PieceType::PAWN:
        generatePawnMoves(piece, checkSavers);
        break;
    case KNIGHT:
        generateKnightMoves(piece, checkSavers);
        break;
    case PieceType::KING:
        generateKingMoves(piece, checkSavers);
        break;
    default:
        generateOthersMoves(piece, checkSavers);
        break;
    }
    updatedPieces.insert(piece->getID());
}

void MoveGenerator::generatePieceMoves(const Square& dir, Piece* piece, vector<Square>* checkSavers)
{
    removeMoves(piece->getID(), dir);
    switch (piece->getType())
    {
    case PieceType::PAWN:
        generatePawnMoves(dir, piece, checkSavers);
        break;
    case KNIGHT:
        generateKnightMoves(dir, piece, checkSavers);
        break;
    case PieceType::KING:
        generateKingMoves(piece, checkSavers);
        break;
    default:
        generateOthersMoves(dir, piece, checkSavers);
        break;
    }
}

void MoveGenerator::dynamicUpdateFromSquare(const Square& square, std::unordered_set<int>& foundPieces,
                                            vector<Square>** checkCovers)
{
    vector<int>* copied = new vector<int>(squareDependable[square.y][square.x]);
    for (const int& id : (*copied))
    {
        if (updatedPieces.find(id) == updatedPieces.end())
        {
            Piece* piece = config->getPiece(id);
            if (BoardConfig::isKnight(piece))
                generatePieceMoves(square - piece->getPos(), piece, checkCovers[(int)piece->getColor()]);
            else
                generatePieceMoves(BoardConfig::toDirectionalVector2(piece->getPos(), square), piece, checkCovers[(int)piece->getColor()]);
        }
    }
    delete copied;
}

void MoveGenerator::generateAllMoves(Side side)
{
    int piecesID = (int)(side) * 16;
    vector<Square>* checkSavers = config->getSquaresCoveringChecks(side);
    for (int i = piecesID; i < piecesID + 16; i++)
    {
        Piece* piece = config->getPiece(i);
        if (piece->isActive())
            generatePieceMoves(piece, checkSavers);
        else
        {
            legalMoves[i].clear();
            attacksOnly[i].clear();
        }
    }
    updatedPieces.clear();
}

void MoveGenerator::updateByMove(int pieceID, const Square& oldPos, const Square& newPos)
{
    if (newPos.x == 9)
    {
        removeMoves(pieceID);
        return;
    }
    Piece* piece = config->getPiece(pieceID);
    Side color = piece->getColor();
    Side oppositionColor = opposition(color);
    int sideID = (int)color;
    int oppositionSideID = (sideID + 1) % 2;
    updatedPieces.insert(pieceID);
    bool checkFlags[2] = {false, false};
    vector<Square>* checkCovers[2] = {nullptr, nullptr};
    if (config->isKingChecked(color))
    {
        checkCovers[sideID] = config->getSquaresCoveringChecks(color);
        if (!checkStates[sideID])
        {
            checkStates[sideID] = true;
            checkFlags[sideID] = true;
        }
    }
    else
    {
        if (checkStates[sideID])
        {
            checkStates[sideID] = false;
            checkFlags[sideID] = true;
        }
        if (config->isKingChecked(oppositionColor))
        {
            checkCovers[oppositionSideID] = config->getSquaresCoveringChecks(oppositionColor);
            if (!checkStates[oppositionSideID])
            {
                checkStates[oppositionSideID] = true;
                checkFlags[oppositionSideID] = true;
            }
        }
        else if (checkStates[oppositionSideID])
        {
            checkStates[oppositionSideID] = false;
            checkFlags[oppositionSideID] = true;
        }
    }
    if (oldPos == newPos)
    {
        generatePieceMoves(piece, checkCovers[sideID]);
        return;
    }
    if (oldPos.x < 8)
        dynamicUpdateFromSquare(oldPos, updatedPieces, checkCovers);
    if (newPos.x < 8)
    {
        dynamicUpdateFromSquare(newPos, updatedPieces, checkCovers);
        generatePieceMoves(piece, checkCovers[sideID]);

        if (checkFlags[oppositionSideID])
        {
            int id = oppositionSideID * 16;
            for (int i = id + 1; i < id + 16; i++)
            {
                Piece* foundPiece = config->getPiece(i);
                if (foundPiece->isActive() && updatedPieces.find(i) == updatedPieces.end())
                    generatePieceMoves(foundPiece, checkCovers[oppositionSideID]);
            }
        }
        else if (checkFlags[sideID])
        {
            int id = sideID * 16;
            for (int i = id; i < id + 16; i++)
            {
                Piece* foundPiece = config->getPiece(i);
                if (foundPiece->isActive() && updatedPieces.find(i) == updatedPieces.end())
                    generatePieceMoves(foundPiece, checkCovers[sideID]);
            }
        }
    }
    generatePieceMoves(config->getPiece(sideID * 16), nullptr);
    generatePieceMoves(config->getPiece(oppositionSideID * 16), nullptr);
    updatedPieces.clear();
}




/// Getters & others
void MoveGenerator::showMoves(bool attacks)
{
    for (int i = 0; i < 32; i++)
    {
        for (const Move2& move : legalMoves[i])
        {
             Piece* piece = config->getPiece(move.pieceID);
             cout<<move.pieceID<<", type: "<<(int)move.pieceType<<", oldPos: ("<<piece->getPos().x<<","<<piece->getPos().y<<"), newPos: (";
             cout<<move.targetPos.x<<","<<move.targetPos.y<<"), flags: ";
             move.specialFlag.show();
             cout<<endl;
        }
        if (attacks)
        {
            for (const Move2& move : attacksOnly[i])
            {
                Piece* piece = config->getPiece(move.pieceID);
                cout << move.pieceID << ", type: " << (int)move.pieceType << ", oldPos: (" << piece->getPos().x << "," << piece->getPos().y << "), newPos: (";
                cout << move.targetPos.x << "," << move.targetPos.y << "), flags: ";
                move.specialFlag.show();
                cout << endl;
            }
        }
    }
    cout<<endl;
}

