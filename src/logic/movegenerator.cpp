#include "movegenerator.h"
#include <algorithm>
using std::vector;
using std::unordered_set;
using std::find;
using std::cout;
using std::endl;


MoveGenerator::MoveGenerator(BoardConfig* conf)
    : config(conf)
{
    initializeMoveLists();
}



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
        setListKeys(pieceID, 8, PieceHandlers::getMoveDirections(KNIGHT));
        break;
    case BISHOP:
        setListKeys(pieceID, 4, PieceHandlers::getMoveDirections(BISHOP));
        break;
    case ROOK:
        setListKeys(pieceID, 4, PieceHandlers::getMoveDirections(ROOK));
        break;
    case QUEEN:
        setListKeys(pieceID, 8, PieceHandlers::getMoveDirections(QUEEN));
        break;
    case KING:
        legalMoves[pieceID].reset(1);
        attacksOnly[pieceID].reset(1);
        break;
    }
}

void MoveGenerator::setListKeys(int pieceID, int directionCount, const DirectionsVec& directions)
{
    legalMoves[pieceID].reset(directionCount);
    attacksOnly[pieceID].reset(directionCount);
    for (const Square& dir : directions)
    {
        legalMoves[pieceID].setKey(dir);
        attacksOnly[pieceID].setKey(dir);
    }
}

inline void MoveGenerator::addLegalMove(const Square& dir, int pieceID, PieceType type, const Square& targetPos, int flags)
{
    Move move(pieceID, type, targetPos, flags);
    legalMoves[pieceID].add(dir, move);
    updateObserversByInsertion(move);
    squareDependable[targetPos.y][targetPos.x].push_back(pieceID);
}

inline void MoveGenerator::addPseudoLegalMove(const Square& dir, int pieceID, PieceType type, const Square& targetPos, int flags)
{
    Move move(pieceID, type, targetPos, flags);
    attacksOnly[pieceID].add(dir, move);
    updateObserversByInsertion(move);
    squareDependable[targetPos.y][targetPos.x].push_back(pieceID);
}

void MoveGenerator::removeMoves(int pieceID, const Square& dir)
{
    const vector<Move>& legal = legalMoves[pieceID].getMoves(dir);
    const vector<Move>& attacks = attacksOnly[pieceID].getMoves(dir);
    updateObserversByRemoval(pieceID, legal, true);
    updateObserversByRemoval(pieceID, attacks, false);
    for (const Move& move : legal)
    {
        vector<int>& square = squareDependable[move.targetPos.y][move.targetPos.x];
        vector<int>::iterator it = find(square.begin(), square.end(), pieceID);
        if (it != square.end())
            square.erase(it);
    }
    for (const Move& move : attacks)
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
    for (const Move& move : legalMoves[pieceID])
    {
        vector<int>& square = squareDependable[move.targetPos.y][move.targetPos.x];
        vector<int>::iterator it = find(square.begin(), square.end(), pieceID);
        if (it != square.end())
            square.erase(it);
    }
    for (const Move& move : attacksOnly[pieceID])
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
void MoveGenerator::generatePawnMoves(const Piece* pawn, const vector<Square>* checkSavers)
{
    int pawnID = pawn->getID();
    const Square& currPos = pawn->getPosition();
    const Square& pinVector = config->getPinDirection(pawnID);
    bool pinFlag = pinVector.x < 8 ? true : false;
    if (pawn->getColor() == WHITE)
    {
        if (currPos.x > 0)
            generatePawnMoves(Square(-1, -1), pawn, checkSavers);
        generatePawnMoves(Square(0, -1), pawn, checkSavers);
        if (currPos.x < 7)
            generatePawnMoves(Square(1, -1), pawn, checkSavers);
        if (config->isEnPassantAvailable(pawn))
        {
            Square movementVector(config->getEnPassantLine() - currPos.x, -1);
            Square targetPos = currPos + movementVector;
            if ((!pinFlag || pinVector == movementVector || pinVector == -movementVector) &&
                (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
                addLegalMove(movementVector, pawnID, PAWN, targetPos, 0b1001);
        }
    }
    else
    {
        if (currPos.x > 0)
            generatePawnMoves(Square(-1, 1), pawn, checkSavers);
        generatePawnMoves(Square(0, 1), pawn, checkSavers);
        if (currPos.x < 7)
            generatePawnMoves(Square(1, 1), pawn, checkSavers);
        if (config->isEnPassantAvailable(pawn))
        {
            Square movementVector(config->getEnPassantLine() - currPos.x, 1);
            Square targetPos = currPos + movementVector;
            if ((!pinFlag || pinVector == movementVector || pinVector == -movementVector) &&
                (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
                addLegalMove(movementVector, pawnID, PAWN, targetPos, 0b1001);
        }
    }

}

void MoveGenerator::generatePawnMoves(const Square& dir, const Piece* pawn, const vector<Square>* checkSavers)
{
    int pawnID = pawn->getID();
    const Square& pinVector = config->getPinDirection(pawnID);
    bool pinFlag = pinVector.x < 8 ? true : false;
    if (dir.x == 0)
    {
        bool pinCheck = !pinFlag || pinVector == dir || pinVector == -dir;
        Square targetPos = pawn->getPosition() + dir;
        if (config->getPiece(targetPos) == nullptr && pinCheck &&
            (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
        {
            if (targetPos.y == 0)
            {
                Move move1(pawnID, PieceType::PAWN, targetPos, 0b1000, QUEEN);
                legalMoves[pawnID].add(dir, move1);
                updateObserversByInsertion(move1);
                Move move2(pawnID, PieceType::PAWN, targetPos, 0b1000, KNIGHT);
                legalMoves[pawnID].add(dir, move2);
                updateObserversByInsertion(move2);
                Move move3(pawnID, PieceType::PAWN, targetPos, 0b1000, ROOK);
                legalMoves[pawnID].add(dir, move3);
                updateObserversByInsertion(move3);
                Move move4(pawnID, PieceType::PAWN, targetPos, 0b1000, BISHOP);
                legalMoves[pawnID].add(dir, move4);
                updateObserversByInsertion(move4);
                squareDependable[targetPos.y][targetPos.x].push_back(pawnID);
            }
            else
                addLegalMove(dir, pawnID, PAWN, targetPos, 0b1000);
        }
        else
            addPseudoLegalMove(dir, pawnID, PAWN, targetPos, 0b0000);
        if (pawn->getPosition().y == 6 && pawn->getColor() == WHITE)
        {
            targetPos = pawn->getPosition() + Square(0, -2);
            if (config->getPiece(Square(pawn->getPosition().x, pawn->getPosition().y - 1)) == nullptr &&
                config->getPiece(targetPos) == nullptr && pinCheck &&
                (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
                addLegalMove(dir, pawnID, PAWN, targetPos, 0b1000);
            else
                addPseudoLegalMove(dir, pawnID, PAWN, targetPos, 0b0000);
        }
        else if (pawn->getPosition().y == 1 && pawn->getColor() == BLACK)
        {
            targetPos = pawn->getPosition() + Square(0, 2);
            if (config->getPiece(Square(pawn->getPosition().x, pawn->getPosition().y + 1)) == nullptr &&
                config->getPiece(targetPos) == nullptr && pinCheck &&
                (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
                addLegalMove(dir, pawnID, PAWN, targetPos, 0b1000);
            else
                addPseudoLegalMove(dir, pawnID, PAWN, targetPos, 0b0000);
        }
    }
    else if (!pinFlag || pinVector == dir || pinVector == -dir)
    {
        Square targetPos = pawn->getPosition() + dir;
        if (config->getPiece(targetPos) != nullptr && config->getPiece(targetPos)->getColor() != pawn->getColor() &&
            (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
        {
            if (targetPos.y == 0 || targetPos.y == 7)
            {
                Move move1(pawnID, PieceType::PAWN, targetPos, 0b1100, QUEEN);
                legalMoves[pawnID].add(dir, move1);
                updateObserversByInsertion(move1);
                Move move2(pawnID, PieceType::PAWN, targetPos, 0b1100, KNIGHT);
                legalMoves[pawnID].add(dir, move2);
                updateObserversByInsertion(move2);
                Move move3(pawnID, PieceType::PAWN, targetPos, 0b1100, ROOK);
                legalMoves[pawnID].add(dir, move3);
                updateObserversByInsertion(move3);
                Move move4(pawnID, PieceType::PAWN, targetPos, 0b1100, BISHOP);
                legalMoves[pawnID].add(dir, move4);
                updateObserversByInsertion(move4);
                squareDependable[targetPos.y][targetPos.x].push_back(pawnID);
            }
            else
                addLegalMove(dir, pawnID, PAWN, targetPos, 0b1100);
        }
        else
            addPseudoLegalMove(dir, pawnID, PAWN, targetPos, 0b0100);
    }
}

void MoveGenerator::generateKnightMoves(const Piece* knight, const vector<Square>* checkSavers)
{
    const Square& pinVector = config->getPinDirection(knight->getID());
    if (pinVector.x < 8)
        return;
    int knightID = knight->getID();
    const Square& currPos = knight->getPosition();
    const DirectionsVec& directions = knight->getMoveDirections();
    for (const Square& dir : directions)
    {
        Square targetPos = currPos + dir;
        const Piece* piece = config->getPiece(targetPos);
        if (targetPos.x >= 0 && targetPos.y >= 0 && targetPos.y < 8 && targetPos.x < 8)
        {
            if ((checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()) &&
                (piece == nullptr || piece->getColor() != knight->getColor()))
                addLegalMove(dir, knightID, KNIGHT, targetPos, 0b1100);
            else
                addPseudoLegalMove(dir, knightID, KNIGHT, targetPos, 0b0100);
        }
    }
}

void MoveGenerator::generateKnightMoves(const Square& dir, const Piece* knight, const vector<Square>* checkSavers)
{
    Square targetPos = knight->getPosition() + dir;
    const Piece* piece = config->getPiece(targetPos);
    if ((checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()) &&
        (piece == nullptr || piece->getColor() != knight->getColor()))
        addLegalMove(dir, knight->getID(), KNIGHT, targetPos, 0b1100);
    else
        addPseudoLegalMove(dir, knight->getID(), KNIGHT, targetPos, 0b0100);
}

void MoveGenerator::generateKingMoves(const Piece* king, const vector<Square>* checkSavers)
{
    Side side = king->getColor();
    int kingID = king->getID();
    const Square& currPos = king->getPosition();
    const DirectionsVec& directions = king->getMoveDirections();
    for (const Square& dir : directions)
    {
        Square targetPos = currPos + dir;
        if (targetPos.x >= 0 && targetPos.x < 8 && targetPos.y >= 0 && targetPos.y < 8)
        {
            const Piece* piece = config->getPiece(targetPos);
            if (!config->isSquareChecked(targetPos, opposition[side]) && (piece == nullptr || piece->getColor() != king->getColor()))
            {
                Move move(kingID, PieceType::KING, targetPos, 0b1100);
                legalMoves[kingID].add(dir, move);
                updateObserversByInsertion(move);
            }
            else
            {
                Move move(kingID, PieceType::KING, targetPos, 0b0100);
                attacksOnly[kingID].add(dir, move);
                updateObserversByInsertion(move);
            }
        }
    }

    if (currPos.x > 1 && config->isCastlingAvailable(side, SHORT_CASTLE) > 0)
    {
        Move move(kingID, PieceType::KING, Square(currPos.x + 2, currPos.y), 0b1010);
        legalMoves[kingID].add(Square(-1, 0), move);
        updateObserversByInsertion(move);
    }
    if (currPos.x < 6 && config->isCastlingAvailable(side, LONG_CASTLE) > 0)
    {
        Move move(kingID, PieceType::KING, Square(currPos.x - 2, currPos.y), 0b1010);
        legalMoves[kingID].add(Square(1, 0), move);
        updateObserversByInsertion(move);
    }
}

inline void MoveGenerator::registerMove(const Square& dir, int pieceID, PieceType type, const Square& targetPos,
    const vector<Square>* checkSavers, bool batteryFlag)
{
    if (!batteryFlag && (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
        addLegalMove(dir, pieceID, type, targetPos, 0b1100);
    else
        addPseudoLegalMove(dir, pieceID, type, targetPos, 0b0100);
}

void MoveGenerator::generateOthersMoves(const Piece* other, const vector<Square>* checkSavers)
{
    const Square& pinVector = config->getPinDirection(other->getID());
    Side side = other->getColor();
    int pieceID = other->getID();
    const Square& currPos = other->getPosition();
    const DirectionsVec& directions = other->getMoveDirections();
    int oppositeSideID = (int)opposition[side] * 16;
    for (const Square& dir : directions)
    {
        bool isRookDir = dir.x == 0 || dir.y == 0;
        PieceType type = other->getType();
        if (pinVector.x < 8 && dir != pinVector && dir != -pinVector)
            continue;
        Square targetPos = currPos + dir;
        bool batteryFlag = false;
        const Piece* foundPiece;
        while (BoardManipulation::isCorrectSquare(targetPos))
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
                    addPseudoLegalMove(dir, pieceID, type, targetPos, 0b0100);
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

void MoveGenerator::generateOthersMoves(const Square& dir, const Piece* other, const vector<Square>* checkSavers)
{
    Side side = other->getColor();
    int oppositeSideID = (int)opposition[side] * 16;
    int pieceID = other->getID();
    bool isRookDir = dir.x == 0 || dir.y == 0;
    PieceType type = other->getType();
    Square targetPos = other->getPosition() + dir;
    bool batteryFlag = false;
    const Piece* foundPiece;
    while (BoardManipulation::isCorrectSquare(targetPos))
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
                addPseudoLegalMove(dir, pieceID, type, targetPos, 0b0100);
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
void MoveGenerator::generatePieceMoves(const Piece* piece, const vector<Square>* checkSavers)
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

void MoveGenerator::generatePieceMoves(const Square& dir, const Piece* piece, const vector<Square>* checkSavers)
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
                                            const SquaresVec* const* checkCovers)
{
    vector<int>* copied = new vector<int>(squareDependable[square.y][square.x]);
    for (const int& id : (*copied))
    {
        if (updatedPieces.find(id) == updatedPieces.end())
        {
            const Piece* piece = config->getPiece(id);
            if (piece->getType() == KNIGHT)
                generatePieceMoves(square - piece->getPosition(), piece, checkCovers[piece->getColor()]);
            else
                generatePieceMoves(BoardManipulation::toDirectionalVector(piece->getPosition(), square), piece, checkCovers[piece->getColor()]);
        }
    }
    delete copied;
}

void MoveGenerator::generateNonKingMoves(Side side)
{
    int piecesID = (int)(side) * 16;
    const SquaresVec* checkSavers = config->getSquaresCoveringChecks(side);
    for (int i = piecesID + 1; i < piecesID + 16; i++)
    {
        const Piece* piece = config->getPiece(i);
        if (piece->isActive())
            generatePieceMoves(piece, checkSavers);
        else
            removeMoves(i);
    }
    updatedPieces.clear();
}

void MoveGenerator::generateAllMoves()
{
    config->clearAttacksTable();
    generateNonKingMoves(WHITE);
    generateNonKingMoves(BLACK);
    generatePieceMoves(config->getKing(WHITE), config->getSquaresCoveringChecks(WHITE));
    generatePieceMoves(config->getKing(BLACK), config->getSquaresCoveringChecks(BLACK));
    generatePieceMoves(config->getKing(WHITE), config->getSquaresCoveringChecks(WHITE));
}

void MoveGenerator::updateByMove(int pieceID, const Square& oldPos, const Square& newPos)
{
    if (newPos.x == 9)
    {
        removeMoves(pieceID);
        return;
    }
    const Piece* piece = config->getPiece(pieceID);
    Side color = piece->getColor();
    Side oppositionColor = opposition[color];
    int sideID = (int)color;
    int oppositionSideID = (sideID + 1) % 2;
    updatedPieces.insert(pieceID);
    bool checkFlags[2] = {false, false};
    const SquaresVec* checkCovers[2] = {nullptr, nullptr};
    if (config->isInCheck(color))
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
        if (config->isInCheck(oppositionColor))
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
                const Piece* foundPiece = config->getPiece(i);
                if (foundPiece->isActive() && updatedPieces.find(i) == updatedPieces.end())
                    generatePieceMoves(foundPiece, checkCovers[oppositionSideID]);
            }
        }
        else if (checkFlags[sideID])
        {
            int id = sideID * 16;
            for (int i = id; i < id + 16; i++)
            {
                const Piece* foundPiece = config->getPiece(i);
                if (foundPiece->isActive() && updatedPieces.find(i) == updatedPieces.end())
                    generatePieceMoves(foundPiece, checkCovers[sideID]);
            }
        }
    }
    generatePieceMoves(config->getKing(color), nullptr);
    generatePieceMoves(config->getKing(oppositionColor), nullptr);
    updatedPieces.clear();
}



/// Getters & others
void MoveGenerator::showMoves(bool attacks)
{
    for (int i = 0; i < 32; i++)
    {
        for (const Move& move : legalMoves[i])
        {
             const Piece* piece = config->getPiece(move.pieceID);
             cout<<move.pieceID<<", type: "<<(int)move.pieceType<<", oldPos: ("<<piece->getPosition().x<<","<<piece->getPosition().y<<"), newPos: (";
             cout << move.targetPos.x << "," << move.targetPos.y << "), flags: " << move.flags << std::endl;
             cout<<endl;
        }
        if (attacks)
        {
            for (const Move& move : attacksOnly[i])
            {
                const Piece* piece = config->getPiece(move.pieceID);
                cout << move.pieceID << ", type: " << (int)move.pieceType << ", oldPos: (" << piece->getPosition().x << "," << piece->getPosition().y << "), newPos: (";
                cout << move.targetPos.x << "," << move.targetPos.y << "), flags: " << move.flags << std::endl;
                cout << endl;
            }
        }
    }
    cout<<endl;
}

