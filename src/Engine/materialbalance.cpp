#include "materialbalance.h"
using sf::Vector2i;

// PieceType == KING is used as a flag of not active piece, which is not included in evaluating

constexpr int MaterialBalance::typeWeights[];

void MaterialBalance::clearTables()
{
    totalPieceWeight = 0;
    bishops[0][0] = bishops[1][0] = false;
    bishops[0][1] = bishops[1][1] = false;
    for (int i = 0; i < 5; i++)
    {
        pieceCount[i][0] = 0;
        pieceCount[i][1] = 0;
    }
}

void MaterialBalance::evaluate()
{
    clearTables();
    for (int i = 1; i < 16; i++)
    {
        Piece* piece = config->getPiece(i);
        if (piece->isActive())
        {
            piecesInfo[i] = piece->getType();
            int typeID = (int)piecesInfo[i];
            pieceCount[typeID][0] += 1;
            totalPieceWeight += typeWeights[typeID];
            if (typeID == 2)
                bishops[(piece->getPos().x + piece->getPos().y) % 2][0] = true;
        }
        else
            piecesInfo[i] = PieceType::INACTIVE;
    }
    for (int i = 17; i < 32; i++)
    {
        Piece* piece = config->getPiece(i);
        if (piece->isActive())
        {
            piecesInfo[i] = piece->getType();
            int typeID = (int)piecesInfo[i];
            pieceCount[typeID][1] += 1;
            totalPieceWeight += typeWeights[typeID];
            if (typeID == 2)
                bishops[(piece->getPos().x + piece->getPos().y) % 2][1] = true;
        }
        else
            piecesInfo[i] = PieceType::INACTIVE;
    }
    resetObservers();
}

void MaterialBalance::updateByMove(int pieceID, const Vector2i& oldPos, const Vector2i& newPos)
{
    if (pieceID != 0 && pieceID != 16)
    {
        Piece* piece = config->getPiece(pieceID);
        int typeID = (int)piece->getType();
        int sideID = (int)piece->getColor();
        if (piecesInfo[pieceID] != piece->getType())
        {
            int oldTypeID = (int)piecesInfo[pieceID];
            pieceCount[oldTypeID][sideID] -= 1;
            piecesInfo[pieceID] = piece->getType();
            pieceCount[typeID][sideID] += 1;
            totalPieceWeight += (typeWeights[typeID] - typeWeights[oldTypeID]);
            if (typeID == 2)
                bishops[(newPos.x + newPos.y) % 2][sideID] = true;
            updateObserversByChange(pieceID, oldPos, (PieceType)oldTypeID, piecesInfo[pieceID]);
        }
        else if (newPos.x >= 8)
        {
            pieceCount[typeID][sideID] -= 1;
            piecesInfo[pieceID] = PieceType::INACTIVE;
            totalPieceWeight -= typeWeights[typeID];
            if (typeID == 2)
                bishops[(oldPos.x + oldPos.y) % 2][sideID] = false;
            updateObserversByChange(pieceID, oldPos, piece->getType(), PieceType::INACTIVE);
        }
    }
}

void MaterialBalance::show() const
{
    std::cout<<"Pawns difference: "<<pieceCount[0][0] - pieceCount[0][1]<<std::endl;
    std::cout<<"Knights difference: "<<pieceCount[1][0] - pieceCount[1][1]<<std::endl;
    std::cout<<"Bishops difference: "<<pieceCount[2][0] - pieceCount[2][1]<<std::endl;
    std::cout<<"Rooks difference: "<<pieceCount[3][0] - pieceCount[3][1]<<std::endl;
    std::cout<<"Queens difference: "<<pieceCount[4][0] - pieceCount[4][1]<<std::endl;
    std::cout<<"Game stage points: "<<totalPieceWeight<<std::endl;
}

