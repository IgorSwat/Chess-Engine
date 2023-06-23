#include "squarecontrol.h"
#include "misc.h"
using std::vector;


constexpr int SquareControl::pieceCodes[6];

/// SquareControl methods
SquareControl::SquareControl(MoveGenerator* gen, BoardConfig* cnf) : PositionElement("SquareControl"),
    config(cnf), generator(gen)
{
    connectivity = new Connectivity(config, this);
    clearTables();
    initPointers();
}

SquareControl::~SquareControl() 
{ 
    delete connectivity;
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

const int* SquareControl::countAttackers(const sf::Vector2i& square, COLOR side)
{
    int squareCode = control[square.y][square.x][(int)side];
    attackers[0] = static_cast<int>(squareCode / pieceCodes[PAWN]);
    squareCode -= attackers[0] * pieceCodes[PAWN];
    attackers[1] = static_cast<int>(squareCode / pieceCodes[KNIGHT]);
    squareCode -= attackers[1] * pieceCodes[KNIGHT];
    attackers[2] = static_cast<int>(squareCode / pieceCodes[ROOK]);
    squareCode -= attackers[2] * pieceCodes[ROOK];
    attackers[3] = static_cast<int>(squareCode / pieceCodes[QUEEN]);
    squareCode -= attackers[3] * pieceCodes[QUEEN];
    attackers[4] = static_cast<int>(squareCode / pieceCodes[KING]);
    return attackers;
}


// Static update
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
    connectivity->clearTables();
}

void SquareControl::evaluate()
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
void SquareControl::updateByMove(int pieceID, const sf::Vector2i& oldPos, const sf::Vector2i& newPos)
{
    if (pieceID == 23)
        std::cout << "Break!\n";
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
    connectivity->updateByMove(pieceID, oldPos, newPos);
}

void SquareControl::updateByInsertion(const Move2& move)
{
    if (move.specialFlag.isAttacking())
    {
        int side = move.pieceID < 16 ? 0 : 1;
        int prevState = whoControls(move.targetPos);
        bool prevDefensiveState = isSquareDefended(move.targetPos, COLOR(side));
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
        connectivity->updateByInsertion(move, prevDefensiveState);
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
    connectivity->show();
}




/// Threats
void Connectivity::clearTables()
{
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
            threats[i][j][0] = threats[i][j][1] = 0;
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            for (int k = 0; k < 4; k++)
                totalThreats[i][j][k] = 0;
}

void Connectivity::updateByMove(int pieceID, const sf::Vector2i& oldPos, const sf::Vector2i& newPos)
{
    Piece* piece = config->getPiece(pieceID);
    if (!BoardConfig::isKing(piece))
    {
        COLOR side = pieceID < 16 ? COLOR::WHITE : COLOR::BLACK;
        COLOR opposite = opposition(side);
        int mappedType = mapPieceTypeByValue(piece->getType());
        if (oldPos.x < 8)
        {
            if (threats[oldPos.y][oldPos.x][LOWER_TYPE_ATTACK] > 0)
            {
                threats[oldPos.y][oldPos.x][LOWER_TYPE_ATTACK] = 0;
                threats[oldPos.y][oldPos.x][HANGING_PIECE] = 0;
                totalThreats[(int)opposite][LOWER_TYPE_ATTACK][mappedType]--;
            }
            else if (threats[oldPos.y][oldPos.x][HANGING_PIECE] > 0)
            {
                threats[oldPos.y][oldPos.x][HANGING_PIECE] = 0;
                if (!control->isSquareDefended(oldPos, side))
                    totalThreats[(int)opposite][HANGING_PIECE][mappedType]--;
            }
        }
        if (newPos.x < 8)
        {
            const int* attackers = control->countAttackers(newPos, opposite);
            for (int i = 0; i < mappedType; i++)
                threats[newPos.y][newPos.x][LOWER_TYPE_ATTACK] += attackers[i];
            for (int i = mappedType; i < 5; i++)
                threats[newPos.y][newPos.x][HANGING_PIECE] += attackers[i];
            if (threats[newPos.y][newPos.x][LOWER_TYPE_ATTACK] > 0)
                totalThreats[(int)opposite][LOWER_TYPE_ATTACK][mappedType]++;
            else if (threats[newPos.y][newPos.x][HANGING_PIECE] > 0 && !control->isSquareDefended(newPos, side))
                totalThreats[(int)opposite][HANGING_PIECE][mappedType]++;
        }
    }
}

void Connectivity::updateByInsertion(const Move2& move, const bool& prevDefenseState)
{
    Piece* attackedPiece = config->getPiece(move.targetPos);
    if (attackedPiece != nullptr && !BoardConfig::isKing(attackedPiece))
    {
        COLOR side = move.pieceID < 16 ? COLOR::WHITE : COLOR::BLACK;
        if (attackedPiece->getColor() != side)
        {
            int mappedType = mapPieceTypeByValue(attackedPiece->getType());
            if (mapPieceTypeByValue(move.pieceType) < mappedType)
            {
                threats[move.targetPos.y][move.targetPos.x][LOWER_TYPE_ATTACK]++;
                if (threats[move.targetPos.y][move.targetPos.x][LOWER_TYPE_ATTACK] == 1)
                {
                    totalThreats[(int)side][LOWER_TYPE_ATTACK][mappedType]++;
                    if (threats[move.targetPos.y][move.targetPos.x][HANGING_PIECE] > 0 && !control->isSquareDefended(move.targetPos, attackedPiece->getColor()))
                        totalThreats[(int)side][HANGING_PIECE][mappedType]--;
                }
            }
            else
            {
                threats[move.targetPos.y][move.targetPos.x][HANGING_PIECE]++;
                if (threats[move.targetPos.y][move.targetPos.x][HANGING_PIECE] == 1 && threats[move.targetPos.y][move.targetPos.x][LOWER_TYPE_ATTACK] == 0
                    && !control->isSquareDefended(move.targetPos, attackedPiece->getColor()))
                    totalThreats[(int)side][HANGING_PIECE][mappedType]++;
            }
        }
        else if (!prevDefenseState && isHanging(move.targetPos) && control->isSquareDefended(move.targetPos, side))
            totalThreats[(int)opposition(side)][HANGING_PIECE][mapPieceTypeByValue(attackedPiece->getType())]--;
    }
}

void Connectivity::updateByRemoval(const Move2& move, const COLOR& side, const COLOR& opposite)
{
    Piece* attackedPiece = config->getPiece(move.targetPos);
    if (attackedPiece != nullptr && !BoardConfig::isKing(attackedPiece))
    {
        if (attackedPiece->getColor() != side)
        {
            int mappedType = mapPieceTypeByValue(attackedPiece->getType());
            if (mapPieceTypeByValue(move.pieceType) < mappedType)
            {
                threats[move.targetPos.y][move.targetPos.x][LOWER_TYPE_ATTACK]--;
                if (threats[move.targetPos.y][move.targetPos.x][LOWER_TYPE_ATTACK] == 0)
                {
                    totalThreats[(int)side][LOWER_TYPE_ATTACK][mappedType]--;
                    if (threats[move.targetPos.y][move.targetPos.x][HANGING_PIECE] > 0 && !control->isSquareDefended(move.targetPos, opposite))
                        totalThreats[(int)side][HANGING_PIECE][mappedType]++;
                }
            }
            else
            {
                threats[move.targetPos.y][move.targetPos.x][HANGING_PIECE]--;
                if (threats[move.targetPos.y][move.targetPos.x][HANGING_PIECE] == 0 && threats[move.targetPos.y][move.targetPos.x][LOWER_TYPE_ATTACK] == 0
                    && !control->isSquareDefended(move.targetPos, attackedPiece->getColor()))
                    totalThreats[(int)side][HANGING_PIECE][mappedType]--;
            }
        }
        else if (!control->isSquareDefended(move.targetPos, side) && isHanging(move.targetPos))
            totalThreats[(int)opposite][HANGING_PIECE][mapPieceTypeByValue(attackedPiece->getType())]++;
    }
}

void Connectivity::showThreats(COLOR side) const
{
    using std::cout;
    using std::endl;
    cout << "Hanging pieces: ";
    cout << "(P, " << totalThreats[(int)side][HANGING_PIECE][0] << "), (N/B, " << totalThreats[(int)side][HANGING_PIECE][1] << "), ";
    cout << "(R, " << totalThreats[(int)side][HANGING_PIECE][2] << "), (Q, " << totalThreats[(int)side][HANGING_PIECE][3] << ")\n";
    cout << "Lower type attacks: ";
    cout << "(P, " << totalThreats[(int)side][LOWER_TYPE_ATTACK][0] << "), (N/B, " << totalThreats[(int)side][LOWER_TYPE_ATTACK][1] << "), ";
    cout << "(R, " << totalThreats[(int)side][LOWER_TYPE_ATTACK][2] << "), (Q, " << totalThreats[(int)side][LOWER_TYPE_ATTACK][3] << ")\n";
}

void Connectivity::show() const
{
    using std::cout;
    using std::endl;

    cout << "White threats against black:\n";
    showThreats(COLOR::WHITE);
    cout << endl;
    cout << "Black threats against white:\n";
    showThreats(COLOR::BLACK);
}
