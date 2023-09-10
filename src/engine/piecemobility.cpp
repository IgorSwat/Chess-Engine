#include "piecemobility.h"
#include "materialbalance.h"
using std::vector;


void PieceMobility::setDependencies(MaterialBalance* material)
{
    if (material == nullptr)
    {
        std::string error = "Non existing dependency provided for instance of " + getName() + "\n";
        throw std::logic_error(error);
    }
    this->material = material;
}

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

void PieceMobility::update()
{
    clearTables();
    const MoveList* moveList = generator->getLegalMoves();
    for (int i = 0; i < 32; i++)
    {
        for (const Move& move : moveList[i])
            updateByInsertion(move);
    }
}

void PieceMobility::updateByInsertion(const Move& move)
{
    if (move.hasProperty(Moves::COMMON_FLAG) && move.pieceType != PAWN && move.pieceType != KING)
    {
        Side side = move.pieceID < 16 ? WHITE : BLACK;
        piecesReachingSquare[move.targetPos.y][move.targetPos.x][move.pieceType - 1][side] += 1;
        if (!control->isControlledByPawn(move.targetPos, opposition[side]))
            mobilityCount[move.pieceType - 1][side] += 1;
    }

}

void PieceMobility::updateByAppearance(const Square& pos, Side side)
{
    Side opposite = opposition[side];
    for (int i = 0; i < 4; i++)
        mobilityCount[i][opposite] -= piecesReachingSquare[pos.y][pos.x][i][opposite];
}

void PieceMobility::updateByDisappearance(const Square& pos, Side side)
{
    Side opposite = opposition[side];
    for (int i = 0; i < 4; i++)
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



/// Evaluation
void PieceMobility::initTables()
{
    for (int i = 0; i < 9; i++)
    {
        int opening = interpolation(factors[KNIGHT_MOBILITY_OPENING], i / 8.f, mobilityProgressFunction);
        int endgame = interpolation(factors[KNIGHT_MOBILITY_ENDGAME], i / 8.f, mobilityProgressFunction);
        for (int j = 0; j < 33; j++)
            knightMobility[i][j] = interpolation(opening, endgame, (32.f - j) / 32.f, [](float x)->float {return x; });
    }
    for (int i = 0; i < 14; i++)
    {
        int opening = interpolation(factors[BISHOP_MOBILITY_OPENING], i / 13.f, mobilityProgressFunction);
        int endgame = interpolation(factors[BISHOP_MOBILITY_ENDGAME], i / 13.f, mobilityProgressFunction);
        for (int j = 0; j < 33; j++)
            bishopMobility[i][j] = interpolation(opening, endgame, (32.f - j) / 32.f, [](float x)->float {return x; });
    }
    for (int i = 0; i < 15; i++)
    {
        int opening = interpolation(factors[ROOK_MOBILITY_OPENING], i / 14.f, mobilityProgressFunction);
        int endgame = interpolation(factors[ROOK_MOBILITY_ENDGAME], i / 14.f, mobilityProgressFunction);
        for (int j = 0; j < 33; j++)
            rookMobility[i][j] = interpolation(opening, endgame, (32.f - j) / 32.f, [](float x)->float {return x; });
    }
    for (int i = 0; i < 28; i++)
    {
        int opening = interpolation(factors[QUEEN_MOBILITY_OPENING], i / 27.f, mobilityProgressFunction);
        int endgame = interpolation(factors[QUEEN_MOBILITY_ENDGAME], i / 27.f, mobilityProgressFunction);
        for (int j = 0; j < 33; j++)
            queenMobility[i][j] = interpolation(opening, endgame, (32.f - j) / 32.f, [](float x)->float {return x; });
    }
}

int PieceMobility::evaluate(int& eval, const int& gameStage) const
{
    short startValue = eval;
    if (material->countKnights(WHITE) > 0)
        eval += material->countKnights(WHITE) * knightMobility[getMobility(WHITE, KNIGHT) / material->countKnights(WHITE)][gameStage];
    if (material->countKnights(BLACK) > 0)
        eval -= material->countKnights(BLACK) * knightMobility[getMobility(BLACK, KNIGHT) / material->countKnights(BLACK)][gameStage];
    if (material->countBishops(WHITE) > 0)
        eval += material->countBishops(WHITE) * bishopMobility[getMobility(WHITE, BISHOP) / material->countBishops(WHITE)][gameStage];
    if (material->countBishops(BLACK) > 0)
        eval -= material->countBishops(BLACK) * bishopMobility[getMobility(BLACK, BISHOP) / material->countBishops(BLACK)][gameStage];
    if (material->countRooks(WHITE) > 0)
        eval += material->countRooks(WHITE) * rookMobility[getMobility(WHITE, ROOK) / material->countRooks(WHITE)][gameStage];
    if (material->countRooks(BLACK) > 0)
        eval -= material->countRooks(BLACK) * rookMobility[getMobility(BLACK, ROOK) / material->countRooks(BLACK)][gameStage];
    if (material->countQueens(WHITE) > 0)
        eval += material->countQueens(WHITE) * queenMobility[getMobility(WHITE, QUEEN) / material->countQueens(WHITE)][gameStage];
    if (material->countQueens(BLACK) > 0)
        eval -= material->countQueens(BLACK) * queenMobility[getMobility(BLACK, QUEEN) / material->countQueens(BLACK)][gameStage];
    std::cout << "Piece mobility evaluation: " << eval - startValue << std::endl;
    return eval - startValue;
}

