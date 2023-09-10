#include "materialbalance.h"
#include "pawnstructure.h"
#include <functional>
#include <algorithm>

namespace {
    constexpr int typeWeights[7]{ 1, 9, 9, 12, 60, 0, 0 };
    constexpr int maxNumberOfPawns = 16;
}



void MaterialBalance::setDependencies(PawnStructure* structure)
{
    if (structure == nullptr)
    {
        std::string error = "Non existing dependency provided for instance of " + getName() + "\n";
        throw std::logic_error(error);
    }
    this->structure = structure;
}

void MaterialBalance::clearTables()
{
    totalPieceWeights[0] = totalPieceWeights[1] = 0;
    bishops[0][0] = bishops[1][0] = false;
    bishops[0][1] = bishops[1][1] = false;
    for (int i = 0; i < 5; i++)
    {
        pieceCount[i][0] = 0;
        pieceCount[i][1] = 0;
    }
}

void MaterialBalance::update()
{
    clearTables();
    for (int i = 1; i < 16; i++)
    {
        const Piece* piece = config->getPiece(i);
        const Square& pos = piece->getPosition();
        if (piece->isActive())
        {
            piecesInfo[i] = piece->getType();
            PieceType type = piecesInfo[i];
            pieceCount[type][0] += 1;
            totalPieceWeights[0] += typeWeights[type];
            
            if (type == BISHOP)
                bishops[colorsMap[pos.y][pos.x]][0] = true;
        }
        else
            piecesInfo[i] = PieceType::INACTIVE;
    }
    for (int i = 17; i < 32; i++)
    {
        const Piece* piece = config->getPiece(i);
        const Square& pos = piece->getPosition();
        if (piece->isActive())
        {
            piecesInfo[i] = piece->getType();
            PieceType type = piecesInfo[i];
            pieceCount[type][1] += 1;
            totalPieceWeights[1] += typeWeights[type];
            if (type == BISHOP)
                bishops[colorsMap[pos.y][pos.x]][1] = true;
        }
        else
            piecesInfo[i] = PieceType::INACTIVE;
    }
}

void MaterialBalance::updateByMove(int pieceID, const Square& oldPos, const Square& newPos)
{
    if (pieceID != 0 && pieceID != 16)
    {
        const Piece* piece = config->getPiece(pieceID);
        PieceType type = piece->getType();
        Side side = piece->getColor();
        Side opposite = opposition[side];
        if (newPos.x >= 8)
        {
            pieceCount[type][side] -= 1;
            piecesInfo[pieceID] = PieceType::INACTIVE;
            totalPieceWeights[side] -= typeWeights[type];
            if (type == BISHOP)
                bishops[colorsMap[oldPos.y][oldPos.x]][side] -= 1;
        }
        else
        {
            if (piecesInfo[pieceID] != piece->getType())
            {
                PieceType oldType = piecesInfo[pieceID];
                pieceCount[oldType][side] -= 1;
                piecesInfo[pieceID] = piece->getType();
                pieceCount[type][side] += 1;
                totalPieceWeights[side] += (typeWeights[type] - typeWeights[oldType]);
            }
            if (type == BISHOP && oldPos.x >= 8)
                bishops[colorsMap[newPos.y][newPos.x]][side] += 1;
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
    std::cout<<"Game stage points: "<<stageValue()<<std::endl;

    std::cout << "\nIs pawns endgame: " << isPawnsEndgame() << std::endl;
    std::cout << "Is opposite color bishops endgame: " << isOppositeBishopsEndgame() << std::endl;
}



/// Evaluation
void MaterialBalance::initTables()
{
    for (int i = 0; i < 33; i++)
    {
        float w = (32.f - i) / 32.f;
        pawnBaseValue[i] = interpolation(factors[PAWN_BASE_VALUE], w, [](float x)->float {return x; });
        knightDistantPawnsPenalty[i] = interpolation(factors[KNIGHT_DISTANT_PAWNS_PENALTY], w, [](float x)->float {return x; });
        rookBaseValue[i] = interpolation(factors[ROOK_BASE_VALUE], w, [](float x)->float {return x; });
    }
    for (int i = 0; i < 17; i++)
    {
        knightPawnBonus[i] = interpolation(factors[KNIGHT_PAWN_BONUS], i / 16.f, [](float x)->float {return x * x; });
        rookPawnsPenalty[i] = interpolation(factors[ROOK_PAWNS_PENALTY], i / 16.f, [](float x)->float {return x; });
    }
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 33; j++)
        {
            int interpolationValue = interpolation(factors[BISHOP_COLOR_WEAKNESS_BONUS], i / 8.f, [](float x)->float {return x * x; });
            bishopOppositePawnsPenalty[i][j] = interpolation(interpolationValue, -interpolationValue,
                (32.f - j) / 32.f, [](float x)->float {return x; });
        }
    }
}

int MaterialBalance::evaluatePawns(int& value, const int& gameStage) const
{
    short startValue = value;
    value += (countPawns(WHITE) - countPawns(BLACK)) * pawnBaseValue[gameStage];
    return value - startValue;
}

int MaterialBalance::evaluateKnights(int& value, const int& gameStage) const
{
    short startValue = value;
    short whiteKnights = countKnights(WHITE);
    short blackKnights = countKnights(BLACK);
    value += (whiteKnights - blackKnights) * factors[KNIGHT_BASE_VALUE].startValue;
    value += (whiteKnights - blackKnights) * knightPawnBonus[countPawns()];
    if (structure->distantPawnsCheck(BLACK))
        value += whiteKnights * knightDistantPawnsPenalty[gameStage];
    if (structure->distantPawnsCheck(WHITE))
        value -= blackKnights * knightDistantPawnsPenalty[gameStage];
    return value - startValue;
}

int MaterialBalance::evaluateBishops(int& value, const int& gameStage) const
{
    short startValue = value;
    value += (countBishops(WHITE) - countBishops(BLACK)) * factors[BISHOP_BASE_VALUE].startValue;
    if (hasBishopOfColor(WHITE, LIGHT_SQUARE))
    {
        if (hasBishopOfColor(WHITE, DARK_SQUARE))
            value += factors[BISHOP_PAIR_BONUS].startValue;
    }
    if (hasBishopOfColor(WHITE, DARK_SQUARE))
    {
    }
    if (hasBishopOfColor(BLACK, LIGHT_SQUARE))
    {
        if (hasBishopOfColor(BLACK, DARK_SQUARE))
            value -= factors[BISHOP_PAIR_BONUS].startValue;
    }
    if (hasBishopOfColor(BLACK, DARK_SQUARE))
    {
    }
    return value - startValue;
}

int MaterialBalance::evaluateRooks(int& value, const int& gameStage) const
{
    short startValue = value;
    short whiteRooks = countRooks(WHITE);
    short blackRooks = countRooks(BLACK);
    value += (whiteRooks - blackRooks) * rookBaseValue[gameStage];
    value += (whiteRooks - blackRooks) * rookPawnsPenalty[countPawns()];
    return value - startValue;
}

int MaterialBalance::evaluateQueens(int& value, const int& gameStage) const
{
    short startValue = value;
    value += (countQueens(WHITE) - countQueens(BLACK)) * factors[QUEEN_BASE_VALUE].startValue;
    return value - startValue;
}

int MaterialBalance::evaluate(int& eval, const int& gameStage) const
{
    int sum = 0;
    int prevSum = 0;
    sum += evaluatePawns(eval, gameStage);
    std::cout << "Pawns evaluation: " << (sum - prevSum) << std::endl;
    prevSum = sum;
    sum += evaluateKnights(eval, gameStage);
    std::cout << "Knights evaluation: " << (sum - prevSum) << std::endl;
    prevSum = sum;
    sum += evaluateBishops(eval, gameStage);
    std::cout << "Bishops evaluation: " << (sum - prevSum) << std::endl;
    prevSum = sum;
    sum += evaluateRooks(eval, gameStage);
    std::cout << "Rooks evaluation: " << (sum - prevSum) << std::endl;
    prevSum = sum;
    sum += evaluateQueens(eval, gameStage);
    std::cout << "Queens evaluation: " << (sum - prevSum) << std::endl;
    return sum;
}

