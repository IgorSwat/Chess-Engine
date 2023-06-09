#ifndef PIECEMOBILITY_H_INCLUDED
#define PIECEMOBILITY_H_INCLUDED

#include "positionelement.h"
#include "squarecontrol.h"
#include "misc.h"
using std::vector;

class PieceMobility : public PositionElement, public MoveListChangedObserver, public PawnControlChangedObserver
{
private:
    MoveGenerator* generator;
    SquareControl* control;
    // Data tables
    int piecesReachingSquare[8][8][5][2]{ 0 };
    int mobilityCount[5][2] { 0 };
    // Helper functions
    template <typename Container> void updateByRemovalConstruct(int pieceID, Container moves);
public:
    PieceMobility(MoveGenerator* gen, SquareControl* con) : PositionElement("PieceMobility"), generator(gen), control(con) {}
    // Static update
    void clearTables();
    void evaluate() override;
    void reset() override {evaluate();}
    // Dynamic update
    void updateByInsertion(const Move2& move);
    void updateByRemoval(int pieceID, const vector<Move2>& moves) { updateByRemovalConstruct(pieceID, moves); }
    void updateByRemoval(int pieceID, const MoveList& moves) { updateByRemovalConstruct(pieceID, moves); }
    void updateByAppearance(const sf::Vector2i& pos, int side) override;
    void updateByDisappearance(const sf::Vector2i& pos, int side) override;
    int getMobility(COLOR side, PieceType type) {return mobilityCount[mapPieceType(type)][(int)side];}
    // Testing
    void show() const;
};



template <typename Container>
void PieceMobility::updateByRemovalConstruct(int pieceID, Container moves)
{
    for (const Move2& move : moves)
    {
        if (move.specialFlag.isCommon() && move.pieceType != PieceType::PAWN)
        {
            int colorFlag = move.pieceID < 16 ? 0 : 1;
            int type = mapPieceType(move.pieceType);
            piecesReachingSquare[move.targetPos.y][move.targetPos.x][type][colorFlag] -= 1;
            if (!control->isControlledByPawn(move.targetPos, (colorFlag + 1) % 2))
                mobilityCount[type][colorFlag] -= 1;
        }
    }
}

#endif // PIECEMOBILITY_H_INCLUDED
