#include "squarecontrol.h"
#include "../logic/misc.h"
using std::vector;


constexpr int SquareControl::pieceCodes[6];
constexpr int SquareControl::threatsValues[4];

/// SquareControl methods
SquareControl::SquareControl(MoveGenerator* gen, BoardConfig* cnf, const FactorsVec& ftors) : PositionElement("SquareControl", ftors, 3),
    config(cnf), generator(gen)
{
    clearTables();
    initPointers();
    initTables();
}

void SquareControl::initPointers()
{
    for (int i = 0; i < 27; i++)
    {
        counterPointers[i / 8][i % 8][0] = &opponentCampControl[0];
        counterPointers[i / 8][i % 8][1] = &ownCampControl[1];
    }
    for (int i = 3; i < 5; i++)
    {
        for (int j = 3; j < 5; j++)
        {
            counterPointers[i][j][0] = &centerControl[0];
            counterPointers[i][j][1] = &centerControl[1];
        }
    }
    for (int i = 29; i < 32; i++)
    {
        counterPointers[i / 8][i % 8][0] = &opponentCampControl[0];
        counterPointers[i / 8][i % 8][1] = &ownCampControl[1];
    }
    for (int i = 32; i < 35; i++)
    {
        counterPointers[i / 8][i % 8][0] = &ownCampControl[0];
        counterPointers[i / 8][i % 8][1] = &opponentCampControl[1];
    }
    for (int i = 37; i < 64; i++)
    {
        counterPointers[i / 8][i % 8][0] = &ownCampControl[0];
        counterPointers[i / 8][i % 8][1] = &opponentCampControl[1];
    }
}

inline void SquareControl::updateControlTables(const Square& pos, const int& prevState, const int& currState)
{
    if (prevState != -1)
        *counterPointers[pos.y][pos.x][prevState] -= 1;
    if (currState != -1)
        *counterPointers[pos.y][pos.x][currState] += 1;
}

const int* SquareControl::countAttackers(const Square& square, Side side, PieceType typeBoundary)
{
    int squareCode = control[square.y][square.x][side];
    for (int i = 0; i < typeBoundary; i++)
    {
        attackers[i] = static_cast<int>(squareCode / pieceCodes[i]);
        squareCode -= attackers[i] * pieceCodes[i];
    }
    return attackers;
}


// Static update
void SquareControl::clearTables()
{
    config->clearAttacksTable();
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
            control[i][j][0] = control[i][j][1] = 0;
    }
    ownCampControl[0] = ownCampControl[1] = 0;
    opponentCampControl[0] = opponentCampControl[1] = 0;
    centerControl[0] = centerControl[1] = 0;
}

void SquareControl::update()
{
    clearTables();
    const MoveList* legal = generator->getLegalMoves();
    const MoveList* attacks = generator->getPseudoLegalMoves();
    for (int i = 0; i < 32; i++)
    {
        for (const Move2& move : legal[i])
            updateByInsertion(move);
        for (const Move2& move : attacks[i])
            updateByInsertion(move);
        Piece* piece = config->getPiece(i);
        if (piece->isActive())
        {
            int prevState = whoControls(piece->getPos());
            control[piece->getPos().y][piece->getPos().x][(int)piece->getColor()] += 1;
            int currState = whoControls(piece->getPos());
            if (prevState != currState)
                updateControlTables(piece->getPos(), prevState, currState);
        }
    }
}


// Dynamic update
void SquareControl::updateByMove(int pieceID, const Square& oldPos, const Square& newPos)
{
    int side = pieceID < 16 ? 0 : 1;
    if (oldPos.x < 8)
    {
        int prevState = whoControls(oldPos);
        control[oldPos.y][oldPos.x][side] -= 1;
        int currState = whoControls(oldPos);
        if (currState != prevState)
            updateControlTables(oldPos, prevState, currState);
    }
    if (newPos.x < 8)
    {
        int prevState = whoControls(newPos);
        control[newPos.y][newPos.x][side] += 1;
        int currState = whoControls(newPos);
        if (currState != prevState)
            updateControlTables(newPos, prevState, currState);
    }
}

void SquareControl::updateByInsertion(const Move2& move)
{
    if (move.specialFlag.isAttacking() && (move.promotionFlag == PAWN || move.promotionFlag == QUEEN))
    {
        Side side = move.pieceID < 16 ? WHITE : BLACK;
        int prevState = whoControls(move.targetPos);
        bool prevDefensiveState = isSquareDefended(move.targetPos, Side(side));
        control[move.targetPos.y][move.targetPos.x][side] += pieceCodes[(int)move.pieceType];
        int currState = whoControls(move.targetPos);
        if (currState != prevState)
            updateControlTables(move.targetPos, prevState, currState);
        if (move.pieceType == PieceType::PAWN && countPawnAttacks(move.targetPos, (Side)side) == 1)
            updateObserversByAppearance(move.targetPos, side);
        config->addAttack(move.targetPos,side);
    }
}


// Testing
void SquareControl::show() const
{
    std::cout<<"Square control statistics (white VS black):\n";
    std::cout<<"Own camp square control: "<<ownCampControl[0]<<"   |   "<<ownCampControl[1]<<std::endl;
    std::cout<<"Opponent`s camp square control: "<<opponentCampControl[0]<<"   |   "<<opponentCampControl[1]<<std::endl;
    std::cout<<"Central squares control: "<<centerControl[0]<<"   |   "<<centerControl[1]<<std::endl;
    std::cout<<"-----------------\n";
    std::cout<<"White controls: ";
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (whoControls(j, i) == 0)
                std::cout<<"("<<j<<","<<i<<") ";
        }
    }
    std::cout<<std::endl;
    std::cout<<"Black controls: ";
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (whoControls(j, i) == 1)
                std::cout<<"("<<j<<","<<i<<") ";
        }
    }
    std::cout << std::endl << std::endl;
}




/// Evaluation
void SquareControl::initTables()
{
    for (int i = 0; i < 33; i++)
    {
        float w = (32.f - i) / 32.f;
        ownCampControlValue[i] = interpolation(factors[OWN_CAMP_CONTROL], w, [](float x)->float {return x; });
        oppositeCampControlValue[i] = interpolation(factors[OPPONENT_CAMP_CONTROL], w, [](float x)->float {return x; });
        centerControlValue[i] = interpolation(factors[CENTER_CONTROL], w, [](float x)->float {return x; });
    }
}

int SquareControl::evaluateSquareControl(int& eval, const int& gameStage) const
{
    int startValue = eval;

    eval += ownCampControlDifference() * ownCampControlValue[gameStage];
    eval += opponentCampControlDifference() * oppositeCampControlValue[gameStage];
    eval += centerControlDifference() * centerControlValue[gameStage];

    return eval - startValue;
}

int SquareControl::evaluate(int& eval, const int& gameStage) const
{
    int sum = 0;
    int prevSum = 0;
    sum += evaluateSquareControl(eval, gameStage);
    std::cout << "Square control evaluation: " << (sum - prevSum) << std::endl;
    return sum;
}
