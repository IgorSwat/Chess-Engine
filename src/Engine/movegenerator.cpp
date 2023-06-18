#include "movegenerator.h"
#include <algorithm>
using sf::Vector2i;
using std::vector;
using std::unordered_set;
using std::find;
using std::cout;
using std::endl;

/// Helper functions
inline void MoveGenerator::addLegalMove(int pieceID, PieceType type, const sf::Vector2i& targetPos, const MoveMask& mask)
{
    legalMoves[pieceID].push_back(Move2(pieceID, type, targetPos, mask));
    updateObserversByInsertion(legalMoves[pieceID].back());
    squareDependable[targetPos.y][targetPos.x].push_back(pieceID);
}

inline void MoveGenerator::addPseudoLegalMove(int pieceID, PieceType type, const sf::Vector2i& targetPos, const MoveMask& mask)
{
    attacksOnly[pieceID].push_back(Move2(pieceID, type, targetPos, mask));
    updateObserversByInsertion(attacksOnly[pieceID].back());
    squareDependable[targetPos.y][targetPos.x].push_back(pieceID);
}

void MoveGenerator::removeMoves(int pieceID)
{
    updateObserversByRemoval(legalMoves[pieceID]);
    updateObserversByRemoval(attacksOnly[pieceID]);
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
void MoveGenerator::generatePawnMoves(Piece* pawn, vector<sf::Vector2i>* checkSavers)
{
    int pawnID = pawn->getID();
    const Vector2i& currPos = pawn->getPos();
    const Vector2i& pinVector = config->isPinned2(pawn);
    bool pinFlag = pinVector.x < 8 ? true : false;
    if (pawn->getColor() == COLOR::WHITE)
    {
        Vector2i targetPos = currPos + Vector2i(-1, -1);
        if (currPos.x > 0 && (!pinFlag || pinVector == Vector2i(-1, -1)))
        {
            if (config->getPiece(targetPos) != nullptr && config->getPiece(targetPos)->getColor() == COLOR::BLACK &&
                (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
            {
                if (targetPos.y == 0)
                {
                    legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK, PieceType::QUEEN));
                    updateObserversByInsertion(legalMoves[pawnID].back());
                    legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK, PieceType::KNIGHT));
                    updateObserversByInsertion(legalMoves[pawnID].back());
                    legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK, PieceType::ROOK));
                    updateObserversByInsertion(legalMoves[pawnID].back());
                    legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK, PieceType::BISHOP));
                    updateObserversByInsertion(legalMoves[pawnID].back());
                    squareDependable[targetPos.y][targetPos.x].push_back(pawnID);
                }
                else
                    addLegalMove(pawnID, PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK);
            }
            else
                addPseudoLegalMove(pawnID, PAWN, targetPos, MoveType::ATTACK);
        }
        targetPos = currPos + Vector2i(1, -1);
        if (currPos.x < 7 && (!pinFlag || pinVector == Vector2i(1, -1)))
        {
            if (config->getPiece(targetPos) != nullptr && config->getPiece(targetPos)->getColor() == COLOR::BLACK &&
                (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
            {
                if (targetPos.y == 0)
                {
                    legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK, PieceType::QUEEN));
                    updateObserversByInsertion(legalMoves[pawnID].back());
                    legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK, PieceType::KNIGHT));
                    updateObserversByInsertion(legalMoves[pawnID].back());
                    legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK, PieceType::ROOK));
                    updateObserversByInsertion(legalMoves[pawnID].back());
                    legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK, PieceType::BISHOP));
                    updateObserversByInsertion(legalMoves[pawnID].back());
                    squareDependable[targetPos.y][targetPos.x].push_back(pawnID);
                }
                else
                    addLegalMove(pawnID, PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK);
            }
            else
                addPseudoLegalMove(pawnID, PAWN, targetPos, MoveType::ATTACK);
        }
        targetPos = currPos + Vector2i(0, -1);
        if (config->getPiece(targetPos) == nullptr &&
            (!pinFlag || pinVector == Vector2i(0, -1)) &&
            (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
        {
            if (targetPos.y == 0)
            {
                legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON, PieceType::QUEEN));
                updateObserversByInsertion(legalMoves[pawnID].back());
                legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON, PieceType::KNIGHT));
                updateObserversByInsertion(legalMoves[pawnID].back());
                legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON, PieceType::ROOK));
                updateObserversByInsertion(legalMoves[pawnID].back());
                legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON, PieceType::BISHOP));
                updateObserversByInsertion(legalMoves[pawnID].back());
                squareDependable[targetPos.y][targetPos.x].push_back(pawnID);
            }
            else
                addLegalMove(pawnID, PAWN, targetPos, MoveType::COMMON);
        }
        else
            addPseudoLegalMove(pawnID, PAWN, targetPos, MoveType::NONE);
        targetPos = currPos + Vector2i(0, -2);
        if (currPos.y == 6)
        {
            if (config->getPiece(Vector2i(currPos.x, currPos.y - 1)) == nullptr &&
                config->getPiece(targetPos) == nullptr &&
                (!pinFlag || pinVector == Vector2i(0, -1)) &&
                (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
                addLegalMove(pawnID, PAWN, targetPos, MoveType::COMMON);
            else
                addPseudoLegalMove(pawnID, PAWN, targetPos, MoveType::NONE);
        }
        if (config->isEnPassantPossible(pawn))
        {
            Vector2i movementVector(config->getEnPassantLine() - currPos.x, -1);
            targetPos = currPos + movementVector;
            if ((!pinFlag || pinVector == movementVector) &&
                (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
                addLegalMove(pawnID, PAWN, targetPos, MoveType::COMMON | MoveType::ENPASSANT);
        }
    }
    else
    {
        Vector2i targetPos = currPos + Vector2i(-1, 1);
        if (currPos.x > 0 && (!pinFlag || pinVector == Vector2i(-1, 1)))
        {
            if (config->getPiece(targetPos) != nullptr && config->getPiece(targetPos)->getColor() == COLOR::WHITE &&
                (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
            {
                if (targetPos.y == 7)
                {
                    legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK, PieceType::QUEEN));
                    updateObserversByInsertion(legalMoves[pawnID].back());
                    legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK, PieceType::KNIGHT));
                    updateObserversByInsertion(legalMoves[pawnID].back());
                    legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK, PieceType::ROOK));
                    updateObserversByInsertion(legalMoves[pawnID].back());
                    legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK, PieceType::BISHOP));
                    updateObserversByInsertion(legalMoves[pawnID].back());
                    squareDependable[targetPos.y][targetPos.x].push_back(pawnID);
                }
                else
                    addLegalMove(pawnID, PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK);
            }
            else
                addPseudoLegalMove(pawnID, PAWN, targetPos, MoveType::ATTACK);

        }
        targetPos = currPos + Vector2i(1, 1);
        if (currPos.x < 7 && (!pinFlag || pinVector == Vector2i(1, 1)))
        {
            if (config->getPiece(targetPos) != nullptr && config->getPiece(targetPos)->getColor() == COLOR::WHITE &&
                (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
            {
                if (targetPos.y == 7)
                {
                    legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK, PieceType::QUEEN));
                    updateObserversByInsertion(legalMoves[pawnID].back());
                    legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK, PieceType::KNIGHT));
                    updateObserversByInsertion(legalMoves[pawnID].back());
                    legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK, PieceType::ROOK));
                    updateObserversByInsertion(legalMoves[pawnID].back());
                    legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK, PieceType::BISHOP));
                    updateObserversByInsertion(legalMoves[pawnID].back());
                    squareDependable[targetPos.y][targetPos.x].push_back(pawnID);
                }
                else
                    addLegalMove(pawnID, PAWN, targetPos, MoveType::COMMON | MoveType::ATTACK);
            }
            else
                addPseudoLegalMove(pawnID, PAWN, targetPos, MoveType::ATTACK);
        }
        targetPos = currPos + Vector2i(0, 1);
        if (config->getPiece(targetPos) == nullptr &&
            (!pinFlag || pinVector == Vector2i(0, 1)) &&
            (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
        {
            if (targetPos.y == 7)
            {
                legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON, PieceType::QUEEN));
                updateObserversByInsertion(legalMoves[pawnID].back());
                legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON, PieceType::KNIGHT));
                updateObserversByInsertion(legalMoves[pawnID].back());
                legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON, PieceType::ROOK));
                updateObserversByInsertion(legalMoves[pawnID].back());
                legalMoves[pawnID].push_back(Move2(pawnID, PieceType::PAWN, targetPos, MoveType::COMMON, PieceType::BISHOP));
                updateObserversByInsertion(legalMoves[pawnID].back());
                squareDependable[targetPos.y][targetPos.x].push_back(pawnID);
            }
            else
                addLegalMove(pawnID, PAWN, targetPos, MoveType::COMMON);
        }
        else
            addPseudoLegalMove(pawnID, PAWN, targetPos, MoveType::NONE);
        targetPos = currPos + Vector2i(0, 2);
        if (currPos.y == 1)
        {
            if (config->getPiece(Vector2i(currPos.x, currPos.y + 1)) == nullptr &&
                config->getPiece(targetPos) == nullptr &&
                (!pinFlag || pinVector == Vector2i(0, 1)) &&
                (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
                addLegalMove(pawnID, PAWN, targetPos, MoveType::COMMON);
            else
                addPseudoLegalMove(pawnID, PAWN, targetPos, MoveType::NONE);
        }
        if (config->isEnPassantPossible(pawn))
        {
            Vector2i movementVector(config->getEnPassantLine() - currPos.x, 1);
            targetPos = currPos + movementVector;
            if ((!pinFlag || pinVector == movementVector) &&
                (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
                addLegalMove(pawnID, PAWN, targetPos, MoveType::COMMON | MoveType::ENPASSANT);
        }
    }
}

void MoveGenerator::generateKnightMoves(Piece* knight, vector<sf::Vector2i>* checkSavers)
{
    const Vector2i& pinVector = config->isPinned2(knight);
    if (pinVector.x < 8)
        return;
    int knightID = knight->getID();
    const Vector2i& currPos = knight->getPos();
    const dirVector& directions = knight->getDirections();
    for (const Vector2i& dir : directions)
    {
        Vector2i targetPos = currPos + dir;
        Piece* piece = config->getPiece(targetPos);
        if (targetPos.x >= 0 && targetPos.y >= 0 && targetPos.y < 8 && targetPos.x < 8)
        {
            if ((checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()) &&
                (piece == nullptr || piece->getColor() != knight->getColor()))
                addLegalMove(knightID, KNIGHT, targetPos, MoveType::COMMON | MoveType::ATTACK);
            else
                addPseudoLegalMove(knightID, KNIGHT, targetPos, MoveType::ATTACK);
        }
    }
}

void MoveGenerator::generateKingMoves(Piece* king, vector<sf::Vector2i>* checkSavers)
{
    COLOR side = king->getColor();
    int kingID = king->getID();
    const Vector2i& currPos = king->getPos();
    const dirVector& directions = king->getDirections();
    for (const Vector2i& dir : directions)
    {
        Vector2i targetPos = currPos + dir;
        if (targetPos.x >= 0 && targetPos.x < 8 && targetPos.y >= 0 && targetPos.y < 8)
        {
            Piece* piece = config->getPiece(targetPos);
            if (!config->isChecked2(targetPos, ++side) && (piece == nullptr || piece->getColor() != king->getColor()))
            {
                legalMoves[kingID].push_back(Move2(kingID, PieceType::KING, targetPos, MoveType::COMMON | MoveType::ATTACK));
                updateObserversByInsertion(legalMoves[kingID].back());
            }
            else
            {
               attacksOnly[kingID].push_back(Move2(kingID, PieceType::KING, targetPos, MoveType::ATTACK));
               updateObserversByInsertion(attacksOnly[kingID].back());
            }
        }
    }
    if (currPos.x > 1 && config->isCastlingAvailable2(king, Vector2i(-2, 0)) > 0)
    {
        legalMoves[kingID].push_back(Move2(kingID, PieceType::KING, Vector2i(currPos.x - 2, currPos.y), MoveType::COMMON | MoveType::CASTLE));
        updateObserversByInsertion(legalMoves[kingID].back());
    }
    if (currPos.x < 6 && config->isCastlingAvailable2(king, Vector2i(2, 0)) > 0)
    {
        legalMoves[kingID].push_back(Move2(kingID, PieceType::KING, Vector2i(currPos.x + 2, currPos.y), MoveType::COMMON | MoveType::CASTLE));
        updateObserversByInsertion(legalMoves[kingID].back());
    }
}

inline void MoveGenerator::registerMove(int pieceID, PieceType type, const sf::Vector2i& targetPos,
    vector<sf::Vector2i>* checkSavers, bool batteryFlag)
{
    if (!batteryFlag && (checkSavers == nullptr || std::find(checkSavers->begin(), checkSavers->end(), targetPos) != checkSavers->end()))
        addLegalMove(pieceID, type, targetPos, MoveType::COMMON | MoveType::ATTACK);
    else
        addPseudoLegalMove(pieceID, type, targetPos, MoveType::ATTACK);
}

void MoveGenerator::generateOthersMoves(Piece* other, vector<sf::Vector2i>* checkSavers)
{
    const Vector2i& pinVector = config->isPinned2(other);
    COLOR side = other->getColor();
    int pieceID = other->getID();
    const Vector2i& currPos = other->getPos();
    const dirVector& directions = other->getDirections();
    int oppositeSideID = (int)++side * 16;
    for (const Vector2i& dir : directions)
    {
        bool isRookDir = dir.x == 0 || dir.y == 0;
        PieceType type = other->getType();
        if (pinVector.x < 8 && dir != pinVector && dir != -pinVector)
            continue;
        Vector2i targetPos = currPos + dir;
        bool batteryFlag = false;
        Piece* foundPiece;
        while (isCorrectSquare(targetPos))
        {
            foundPiece = config->getPiece(targetPos);
            if (foundPiece != nullptr)
            {
                if (foundPiece->getColor() != side)
                {
                    registerMove(pieceID, type, targetPos, checkSavers, batteryFlag);
                    if (foundPiece->getID() == oppositeSideID)
                        batteryFlag = true;
                    else
                        break;
                }
                else
                {
                    addPseudoLegalMove(pieceID, type, targetPos, MoveType::ATTACK);
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
            registerMove(pieceID, type, targetPos, checkSavers, batteryFlag);
            targetPos = targetPos + dir;
        }
    }
}




/// Generic move generators
void MoveGenerator::generatePieceMoves(Piece* piece, vector<sf::Vector2i>* checkSavers)
{
    int pieceID = piece->getID();
    removeMoves(pieceID);
    switch (piece->getType())
    {
    case PieceType::PAWN:
        generatePawnMoves(piece, checkSavers);
        break;
    case PieceType::KNIGHT:
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

void MoveGenerator::dynamicUpdateFromSquare(const sf::Vector2i& square, std::unordered_set<int>& foundPieces,
                                            vector<sf::Vector2i>** checkCovers)
{
    vector<int>* copied = new vector<int>(squareDependable[square.y][square.x]);
    for (const int& id : (*copied))
    {
        if (updatedPieces.find(id) == updatedPieces.end())
        {
            Piece* piece = config->getPiece(id);
            generatePieceMoves(piece, checkCovers[(int)piece->getColor()]);
            updatedPieces.insert(id);
        }
    }
    delete copied;
}

void MoveGenerator::generateAllMoves(COLOR side)
{
    int piecesID = (int)(side) * 16;
    vector<Vector2i>* checkSavers = config->getSquaresCoveringChecks(side);
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

void MoveGenerator::updateByMove(int pieceID, const sf::Vector2i& oldPos, const sf::Vector2i& newPos)
{
    if (newPos.x == 9)
    {
        removeMoves(pieceID);
        return;
    }
    Piece* piece = config->getPiece(pieceID);
    COLOR color = piece->getColor();
    COLOR oppositionColor = ++color;
    int sideID = (int)color;
    int oppositionSideID = (sideID + 1) % 2;
    updatedPieces.insert(pieceID);
    bool checkFlags[2] = {false, false};
    vector<sf::Vector2i>* checkCovers[2] = {nullptr, nullptr};
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
void MoveGenerator::showMoves(bool attacks) const
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

