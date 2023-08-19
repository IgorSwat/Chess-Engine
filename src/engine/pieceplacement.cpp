#include "pieceplacement.h"
#include <algorithm>
#include <functional>
#include <cmath>

namespace {
    constexpr int bishopFianchettoTable[8][8]{
        {1, 0, 0, 0, 0, 0, 0, 1},
        {0, 2, 0, 0, 0, 0, 2, 0},
        {1, 0, 1, 0, 0, 1, 0, 1},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {1, 0, 1, 0, 0, 1, 0, 1},
        {0, 2, 0, 0, 0, 0, 2, 0},
        {1, 0, 0, 0, 0, 0, 0, 1}
    };

    constexpr int distanceToCenterTable[8][8]{
        {6, 5, 4, 3, 3, 4, 5, 6},
        {5, 4, 3, 2, 2, 3, 4, 5},
        {4, 3, 2, 1, 1, 2, 3, 4},
        {3, 2, 1, 0, 0, 1, 2, 3},
        {3, 2, 1, 0, 0, 1, 2, 3},
        {4, 3, 2, 1, 1, 2, 3, 4},
        {5, 4, 3, 2, 2, 3, 4, 5},
        {6, 5, 4, 3, 3, 4, 5, 6}
    };

    constexpr int rowSidesTable[8][8]{
        {1, 1, 1, 1, 2, 2, 2, 2},
        {3, 3, 3, 3, 4, 4, 4, 4},
        {5, 5, 5, 5, 6, 6, 6, 6},
        {7, 7, 7, 7, 8, 8, 8, 8},
        {9, 9, 9, 9, 10, 10, 10, 10},
        {11, 11, 11, 11, 12, 12, 12, 12},
        {13, 13, 13, 13, 14, 14, 14, 14},
        {15, 15, 15, 15, 16, 16, 16, 16}
    };

    constexpr int outpostLowerBounds[2] { 1, 3 };
    constexpr int outpostUpperBounds[2] { 4, 6 };
    constexpr int outpostRankWeights[8]{ 2, 3, 4, 4, 4, 4, 3, 2 };
    constexpr int maxOutpostWeight = 4;

    std::function<bool(const int&, const int&)> isUnderPawnChain[2]{
        [](const int& targetSum, const int& comparisionSum)->bool {return targetSum >= comparisionSum; },
        [](const int& targetSum, const int& comparisionSum)->bool {return targetSum <= comparisionSum; }
    };

    std::function<bool(const int&, const int&)> isBehindOutpostSquare[2]{
        [](const int& outpostRow, const int& otherRow)->bool {return otherRow >= outpostRow; },
        [](const int& outpostRow, const int& otherRow)->bool {return otherRow <= outpostRow; }
    };
}



bool PiecePlacement::isBishopBad(Side side, const Square& square) const
{
    SquareColors color = colorsMap[square.y][square.x];
    return isUnderPawnChain[side](structure->pawnsAtColor(side, color) * square.y, structure->getSumOfPawnRows(side, color));
}

OutpostTypes PiecePlacement::checkForOutpost(Side side, Side opposite, const Square& square) const
{
    if (control->isControlledByPawn(square, side) && !control->isControlledByPawn(square, opposite) &&
        square.y >= outpostLowerBounds[side] && square.y <= outpostUpperBounds[side])
    {
        if ((square.x == 0 || isBehindOutpostSquare[side](square.y, structure->bottomPawnFromColumn(opposite, square.x - 1))) &&
            (square.x == 7 || isBehindOutpostSquare[side](square.y, structure->bottomPawnFromColumn(opposite, square.x + 1))))
        {
            if (material->countKnights(opposite) == 0 && !material->hasBishopOfColor(opposite, colorsMap[square.y][square.x]))
                return ETERNAL_OUTPOST;
            else
                return UNCONTESTED_OUTPOST;
        }
        else
            return COMMON_OUTPOST;
    }
    else
        return NO_OUTPOST;
}





/// Evaluation
void PiecePlacement::initTables()
{
    for (int i = 0; i < 33; i++)
    {
        float w = (32.f - i) / 32.f;
        fianchettoPointValue[i] = interpolation(factors[FIANCHETTO_POINT_VALUE], w, [](float x)->float {return x; });
        int commonOutpostInterpolation = interpolation(factors[COMMON_OUTPOST_VALUE], w, [](float x)->float {return x; });
        int uncontestedOutpostInterpolation = interpolation(factors[UNCONTESTED_OUTPOST_VALUE], w, [](float x)->float {return x; });
        int eternalOutpostInterpolation = interpolation(factors[ETERNAL_OUTPOST_VALUE], w, [](float x)->float {return x; });
        rookUndevelopedPenalty[i] = interpolation(factors[UNDEVELOPED_ROOK_PENALTY], w, [](float x)->float {return x; });
        for (int j = 0; j < 8; j++)
        {
            knightOutpostValue[i][j][NO_OUTPOST] = 0;
            knightOutpostValue[i][j][COMMON_OUTPOST] = commonOutpostInterpolation * outpostRankWeights[j] / maxOutpostWeight;
            knightOutpostValue[i][j][UNCONTESTED_OUTPOST] = uncontestedOutpostInterpolation * outpostRankWeights[j] / maxOutpostWeight;
            knightOutpostValue[i][j][ETERNAL_OUTPOST] = eternalOutpostInterpolation * outpostRankWeights[j] / maxOutpostWeight;
            bishopOutpostValue[i][j][NO_OUTPOST] = 0;
            bishopOutpostValue[i][j][COMMON_OUTPOST] = knightOutpostValue[i][j][COMMON_OUTPOST] * 4 / 5;
            bishopOutpostValue[i][j][UNCONTESTED_OUTPOST] = knightOutpostValue[i][j][UNCONTESTED_OUTPOST] * 4 / 5;
            bishopOutpostValue[i][j][ETERNAL_OUTPOST] = knightOutpostValue[i][j][ETERNAL_OUTPOST] * 4 / 5;
        }
        int kingDistanceToCenterStageFirst = interpolation(factors[KING_DISTANCE_TO_CENTER_OPENING].startValue, factors[KING_DISTANCE_TO_CENTER_ENDGAME].startValue,
                                                            w, [](float x)->float {return x; });
        int kingDistanceToCenterStageSecond = interpolation(factors[KING_DISTANCE_TO_CENTER_OPENING].endValue, factors[KING_DISTANCE_TO_CENTER_ENDGAME].endValue,
                                                            w, [](float x)->float {return x; });
        for (int j = 0; j < 7; j++)
        {
            kingDistanceToCenterValue[i][j] = interpolation(kingDistanceToCenterStageSecond, kingDistanceToCenterStageFirst, (6.f - j) / 6.f,
                                                            [](float x)->float {return std::pow(2.f, 6.f * x - 6.f); });
        }
    }
    for (int i = 0; i < 9; i++)
    {
        badBishopPawnPenalty[i] = interpolation(factors[BAD_BISHOP_PENALTY], i / 8.f, [](float x)->float {return x * x; });
        bishopPawnPenalty[i] = badBishopPawnPenalty[i] / 3;
    }
    rookSemiopenFileBonus = factors[ROOK_SEMIOPEN_FILE_BONUS].startValue;
    rookOpenFileBonus = factors[ROOK_OPEN_FILE_BONUS].startValue;
}

template<> void PiecePlacement::evaluatePiecesPlacement<KNIGHT>(int& eval, const int& gameStage) const
{
    int startValue = eval;
    for (const Square& square : config->getPiecePlacement(WHITE, KNIGHT))
        eval += knightOutpostValue[gameStage][square.x][checkForOutpost(WHITE, BLACK, square)];
    for (const Square& square : config->getPiecePlacement(BLACK, KNIGHT))
        eval -= knightOutpostValue[gameStage][square.x][checkForOutpost(BLACK, WHITE, square)];
    std::cout << "Knights placement (outposts) evaluation: " << eval - startValue << std::endl;
}

template<> void PiecePlacement::evaluatePiecesPlacement<BISHOP>(int& eval, const int& gameStage) const
{
    int startValue = eval;
    for (const Square& square : config->getPiecePlacement(WHITE, BISHOP))
    {
        eval += bishopFianchettoTable[square.y][square.x] * fianchettoPointValue[gameStage];
        eval += bishopOutpostValue[gameStage][square.x][checkForOutpost(WHITE, BLACK, square)];
        if (isBishopBad(WHITE, square))
            eval += badBishopPawnPenalty[structure->pawnsAtColor(WHITE, colorsMap[square.y][square.x])];
        else
            eval += bishopPawnPenalty[structure->pawnsAtColor(WHITE, colorsMap[square.y][square.x])];
    }
    for (const Square& square : config->getPiecePlacement(BLACK, BISHOP))
    {
        eval -= bishopFianchettoTable[square.y][square.x] * fianchettoPointValue[gameStage];
        eval -= bishopOutpostValue[gameStage][square.x][checkForOutpost(BLACK, WHITE, square)];
        if (isBishopBad(BLACK, square))
            eval -= badBishopPawnPenalty[structure->pawnsAtColor(BLACK, colorsMap[square.y][square.x])];
        else
            eval -= bishopPawnPenalty[structure->pawnsAtColor(BLACK, colorsMap[square.y][square.x])];
    }
    std::cout << "Bishops placement evaluation: " << eval - startValue << std::endl;
}

template<> void PiecePlacement::evaluatePiecesPlacement<ROOK>(int& eval, const int& gameStage) const
{
    int startValue = eval;
    const Square& whiteKingPos = config->getKing(WHITE)->getPos();
    for (const Square& square : config->getPiecePlacement(WHITE, ROOK))
    {
        if (structure->isFileOpen(square.x))
            eval += rookOpenFileBonus;
        else if (square.y <= structure->upperPawnRow(WHITE, square.x))
            eval += rookSemiopenFileBonus;
        else if (rowSidesTable[square.y][square.x] == rowSidesTable[whiteKingPos.y][whiteKingPos.x] &&
            distanceToCenterTable[square.y][square.x] > distanceToCenterTable[whiteKingPos.y][whiteKingPos.x])
            eval += rookUndevelopedPenalty[gameStage];
    }
    const Square& blackKingPos = config->getKing(BLACK)->getPos();
    for (const Square& square : config->getPiecePlacement(BLACK, ROOK))
    {
        if (structure->isFileOpen(square.x))
            eval -= rookOpenFileBonus;
        else if (square.y >= structure->upperPawnRow(BLACK, square.x))
            eval -= rookSemiopenFileBonus;
        else if (rowSidesTable[square.y][square.x] == rowSidesTable[blackKingPos.y][blackKingPos.x] &&
            distanceToCenterTable[square.y][square.x] > distanceToCenterTable[whiteKingPos.y][whiteKingPos.x])
            eval -= rookUndevelopedPenalty[gameStage];
    }
    std::cout << "Rooks placement evaluation: " << eval - startValue << std::endl;
}

template<> void PiecePlacement::evaluatePiecesPlacement<KING>(int& eval, const int& gameStage) const
{
    int startValue = eval;
    const Square& whiteKingPos = config->getKing(WHITE)->getPos();
    eval += kingDistanceToCenterValue[gameStage][distanceToCenterTable[whiteKingPos.y][whiteKingPos.x]];
    const Square& blackKingPos = config->getKing(BLACK)->getPos();
    eval -= kingDistanceToCenterValue[gameStage][distanceToCenterTable[blackKingPos.y][blackKingPos.x]];
    std::cout << "Kings placement evaluation: " << eval - startValue << std::endl;
}

int PiecePlacement::evaluate(int& eval, const int& gameStage) const
{
    int startValue = eval;
    evaluatePiecesPlacement<KNIGHT>(eval, gameStage);
    evaluatePiecesPlacement<BISHOP>(eval, gameStage);
    evaluatePiecesPlacement<ROOK>(eval, gameStage);
    evaluatePiecesPlacement<KING>(eval, gameStage);
    return eval - startValue;
}

