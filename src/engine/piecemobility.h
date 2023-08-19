#ifndef PIECEMOBILITY_H_INCLUDED
#define PIECEMOBILITY_H_INCLUDED

#include "positionelement.h"
#include "squarecontrol.h"
#include "../logic/misc.h"
using std::vector;

class MaterialBalance;

class PieceMobility : public PositionElement, public MoveListChangedObserver, public PawnControlChangedObserver
{
private:
    BoardConfig* config;
    MoveGenerator* generator;
    SquareControl* control;
    // Data tables
    int piecesReachingSquare[8][8][5][2]{ 0 };
    int mobilityCount[5][2] { 0 };
    // Evaluation dependencies
    MaterialBalance* material = nullptr;
    // Evaluation elements
    int knightMobility[9][33]{ 0 };
    int bishopMobility[14][33]{ 0 };
    int rookMobility[15][33]{ 0 };
    int queenMobility[28][33]{ 0 };
    // Helper functions
    void initTables();
    template <typename Container> void updateByRemovalConstruct(int pieceID, Container moves, bool legal);

    enum Factors { KNIGHT_MOBILITY_OPENING = 0, KNIGHT_MOBILITY_ENDGAME, BISHOP_MOBILITY_OPENING, BISHOP_MOBILITY_ENDGAME,
        ROOK_MOBILITY_OPENING, ROOK_MOBILITY_ENDGAME, QUEEN_MOBILITY_OPENING, QUEEN_MOBILITY_ENDGAME };

public:
    PieceMobility(BoardConfig* cnf, MoveGenerator* gen, SquareControl* con, const FactorsVec& ftors) : PositionElement("PieceMobility", ftors, 8), 
        config(cnf), generator(gen), control(con) {initTables();}
    void setDependencies(MaterialBalance* material);
    // Static update
    void clearTables();
    void update() override;
    int evaluate(int& eval, const int& gameStage) const;
    // Dynamic update
    void updateByInsertion(const Move2& move);
    void updateByRemoval(int pieceID, const vector<Move2>& moves, bool legal) override { updateByRemovalConstruct(pieceID, moves, legal); }
    void updateByRemoval(int pieceID, const MoveList& moves, bool legal) override { updateByRemovalConstruct(pieceID, moves, legal); }
    void updateByAppearance(const Square& pos, Side side) override;
    void updateByDisappearance(const Square& pos, Side side) override;
    int getMobility(Side side, PieceType type) const {return mobilityCount[type - 1][side];}
    // Testing
    void show() const;
};



template <typename Container>
void PieceMobility::updateByRemovalConstruct(int pieceID, Container moves, bool legal)
{
    PieceType type = config->getPiece(pieceID)->getType();
    if (legal && type != PAWN && type != KING)
    {
        Side side = pieceID < 16 ? WHITE : BLACK;
        for (const Move2& move : moves)
        {
            piecesReachingSquare[move.targetPos.y][move.targetPos.x][type - 1][side] -= 1;
            if (!control->isControlledByPawn(move.targetPos, opposition(side)))
                mobilityCount[type - 1][side] -= 1;
        }
    }
}

#endif // PIECEMOBILITY_H_INCLUDED
