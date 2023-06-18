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
    int piecesReachingSquare[8][8][5][2];
    int mobilityCount[5][2];
public:
    PieceMobility(MoveGenerator* gen, SquareControl* con) : PositionElement("PieceMobility"), generator(gen), control(con) {}
    // Static update
    void clearTables();
    void evaluate() override;
    void reset() override {evaluate();}
    // Dynamic update
    void updateByInsertion(const Move2& move) override;
    void updateByRemoval(const vector<Move2>& moves) override;
    void updateByAppearance(const sf::Vector2i& pos, int side) override;
    void updateByDisappearance(const sf::Vector2i& pos, int side) override;
    int getMobility(COLOR side, PieceType type) {return mobilityCount[mapPieceType(type)][(int)side];}
    // Testing
    void show() const;
};

#endif // PIECEMOBILITY_H_INCLUDED
