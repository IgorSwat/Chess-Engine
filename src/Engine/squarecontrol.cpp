#include "squarecontrol.h"
using std::vector;


constexpr int SquareControl::pieceCodes[6];



/// Initialization
SquareControl::SquareControl(MoveGenerator* gen, BoardConfig* cnf) : PositionElement("SquareControl"),
    config(cnf), generator(gen)
{
    clearTables();
    initPointers();
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

inline void SquareControl::updateControlTables(const sf::Vector2i& pos, const int& prevState, const int& currState)
{
    if (prevState != -1)
        *counterPointers[pos.y][pos.x][prevState] -= 1;
    if (currState != -1)
        *counterPointers[pos.y][pos.x][currState] += 1;
}



/// Static update
void SquareControl::clearTables()
{
    config->clearAttacksTable();
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            control[i][j][0] = control[i][j][1] = 0;
            pawnControl[i][j][0] = 0;
            pawnControl[i][j][1] = 0;
        }
    }
    ownCampControl[0] = ownCampControl[1] = 0;
    opponentCampControl[0] = opponentCampControl[1] = 0;
    centerControl[0] = centerControl[1] = 0;
}

void SquareControl::evaluate()
{
    clearTables();
    const MoveList* legal = generator->getLegalMoves();
    const MoveList* attacks = generator->getPseudoLegalMoves();
    for (int i = 0; i < 16; i++)
    {
        for (const Move2& move : legal[i])
        {
            if (move.specialFlag.isAttacking())
            {
                control[move.targetPos.y][move.targetPos.x][0] += pieceCodes[(int)move.pieceType];
                if (move.pieceType == PieceType::PAWN)
                    pawnControl[move.targetPos.y][move.targetPos.x][0] += 1;
                config->addAttack(move.targetPos, 0);
            }
        }
        for (const Move2& move : attacks[i])
        {
            control[move.targetPos.y][move.targetPos.x][0] += pieceCodes[(int)move.pieceType];
            if (move.pieceType == PieceType::PAWN)
                pawnControl[move.targetPos.y][move.targetPos.x][0] += 1;
            config->addAttack(move.targetPos, 0);
        }
        Piece* piece = config->getPiece(i);
        if (piece->isActive())
            control[piece->getPos().y][piece->getPos().x][0] += 1;
    }
    for (int i = 16; i < 32; i++)
    {
        for (const Move2& move : legal[i])
        {
            if (move.specialFlag.isAttacking())
            {
                control[move.targetPos.y][move.targetPos.x][1] += pieceCodes[(int)move.pieceType];
                if (move.pieceType == PieceType::PAWN)
                    pawnControl[move.targetPos.y][move.targetPos.x][1] += 1;
                config->addAttack(move.targetPos, 1);
            }
        }
        for (const Move2& move : attacks[i])
        {
            control[move.targetPos.y][move.targetPos.x][1] += pieceCodes[(int)move.pieceType];
            if (move.pieceType == PieceType::PAWN)
                pawnControl[move.targetPos.y][move.targetPos.x][1] += 1;
            config->addAttack(move.targetPos, 1);
        }
        Piece* piece = config->getPiece(i);
        if (piece->isActive())
            control[piece->getPos().y][piece->getPos().x][1] += 1;
    }
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            int side = whoControls(j, i);
            if (side != -1)
                *counterPointers[i][j][side] += 1;
        }
    }
}



/// Dynamic update
void SquareControl::updateByMove(int pieceID, const sf::Vector2i& oldPos, const sf::Vector2i& newPos)
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
    if (move.specialFlag.isAttacking())
    {
        int side = move.pieceID < 16 ? 0 : 1;
        int prevState = whoControls(move.targetPos);
        control[move.targetPos.y][move.targetPos.x][side] += pieceCodes[(int)move.pieceType];
        int currState = whoControls(move.targetPos);
        if (currState != prevState)
            updateControlTables(move.targetPos, prevState, currState);
        if (move.pieceType == PieceType::PAWN)
        {
            pawnControl[move.targetPos.y][move.targetPos.x][side] += 1;
            if (pawnControl[move.targetPos.y][move.targetPos.x][side] == 1)
                updateObserversByAppearance(move.targetPos, side);
        }
        config->addAttack(move.targetPos,side);
    }
}



/// Testing
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
    std::cout<<std::endl;
}

