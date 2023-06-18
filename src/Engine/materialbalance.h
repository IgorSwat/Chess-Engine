#ifndef MATERIALBALANCE_H_INCLUDED
#define MATERIALBALANCE_H_INCLUDED

#include "positionelement.h"

class StageFactor;

class MaterialBalance : public PositionElement, public ObservableMaterial, public PositionChangedObserver
{
public:
    static constexpr int maxStagePoints = 256;
    static constexpr int maxNumberOfPawns = 16;
private:
    BoardConfig* config;
    int pieceCount[7][2];
    bool bishops[2][2]; // Contains info about colours of each side bishops
    PieceType piecesInfo[32];
    int totalPieceWeight = 0;
    static constexpr int typeWeights[7] {1, 8, 8, 12, 64, 0, 0};
    /// Helper functions
public:
    MaterialBalance(BoardConfig* cnf) : PositionElement("MaterialBalance"), config(cnf) {}
    /// Static update
    void clearTables();
    void evaluate() override;
    void reset() override {evaluate();}
    /// Dynamic update
    void updateByMove(int pieceID, const sf::Vector2i& oldPos, const sf::Vector2i& newPos);
    /// Extra functionalities
    int stageValue() const {return totalPieceWeight;}
    int stageValueNormalised() const {return totalPieceWeight / 8;}
    int countPawns(COLOR side) const {return pieceCount[0][(int)side];}
    int countPawns() const {return pieceCount[0][0] + pieceCount[0][1];}
    int countKnights(COLOR side) const {return pieceCount[1][(int)side];}
    int countBishops(COLOR side) const {return pieceCount[2][(int)side];}
    bool hasBishopOfColor(COLOR side, int squareColor) const {return bishops[squareColor][(int)side];}
    bool hasBishopPair(COLOR side) const {return bishops[0][(int)side] && bishops[1][(int)side];}
    int countRooks(COLOR side) const {return pieceCount[3][(int)side];}
    int countQueens(COLOR side) const {return pieceCount[4][(int)side];}
    int countHeavyPieces(COLOR side) const {return pieceCount[3][(int)side] + pieceCount[4][(int)side];}
    /// Testing
    void show() const;
};

#endif // MATERIALBALANCE_H_INCLUDED
