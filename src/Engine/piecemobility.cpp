#include "piecemobility.h"
using std::vector;

void PieceMobility::clearTables()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            for (int k = 0; k < 5; k++)
                piecesReachingSquare[i][j][k][0] = piecesReachingSquare[i][j][k][1] = 0;
        }
    }
    for (int i = 0; i < 5; i++)
    {
        mobilityCount[i][0] = 0;
        mobilityCount[i][1] = 0;
    }
}

void PieceMobility::evaluate()
{
    clearTables();
    const MoveList* moveList = generator->getLegalMoves();
    for (int i = 0; i < 32; i++)
    {
        for (const Move2& move : moveList[i])
            updateByInsertion(move);
    }
}

void PieceMobility::updateByInsertion(const Move2& move)
{
    if (move.specialFlag.isCommon() && move.pieceType != PieceType::PAWN)
    {
        int colorFlag = move.pieceID < 16 ? 0 : 1;
        int type = mapPieceType(move.pieceType);
        piecesReachingSquare[move.targetPos.y][move.targetPos.x][type][colorFlag] += 1;
        if (!control->isControlledByPawn(move.targetPos, (colorFlag + 1) % 2))
            mobilityCount[type][colorFlag] += 1;
    }

}

void PieceMobility::updateByAppearance(const sf::Vector2i& pos, int side)
{
    int opposite = (side + 1) % 2;
    for (int i = 0; i < 5; i++)
        mobilityCount[i][opposite] -= piecesReachingSquare[pos.y][pos.x][i][opposite];
}

void PieceMobility::updateByDisappearance(const sf::Vector2i& pos, int side)
{
    int opposite = (side + 1) % 2;
    for (int i = 0; i < 5; i++)
        mobilityCount[i][opposite] += piecesReachingSquare[pos.y][pos.x][i][opposite];
}

void PieceMobility::show() const
{
    std::cout<<"White vs Black mobility per piece type:\n";
    std::cout<<"Knights: "<<mobilityCount[0][0]<<" | "<<mobilityCount[0][1]<<std::endl;
    std::cout<<"Bishops: "<<mobilityCount[1][0]<<" | "<<mobilityCount[1][1]<<std::endl;
    std::cout<<"Rooks: "<<mobilityCount[2][0]<<" | "<<mobilityCount[2][1]<<std::endl;
    std::cout<<"Queens: "<<mobilityCount[3][0]<<" | "<<mobilityCount[3][1]<<std::endl;
    std::cout<<"Kings: "<<mobilityCount[4][0]<<" | "<<mobilityCount[4][1]<<std::endl;
}

