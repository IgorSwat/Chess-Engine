#ifndef MATERIALBALANCE_H_INCLUDED
#define MATERIALBALANCE_H_INCLUDED

#include "positionelement.h"

class PawnStructure;

class MaterialBalance : public PositionElement, public PositionChangedObserver
{
private:
    BoardConfig* config;
    int pieceCount[7][2] { 0 };
    int bishops[2][2] { false };
    PieceType piecesInfo[32] { INACTIVE };
    int totalPieceWeights[2] { 0 };
    static constexpr int maxStagePoints = 256;

    // Evaluation dependencies
    PawnStructure* structure = nullptr;

    // Evaluation elements
    int pawnBaseValue[33]{ 0 };
    int knightPawnBonus[17]{ 0 };
    int knightDistantPawnsPenalty[33]{ 0 };
    int badBishopPawnPenalty[9]{ 0 };
    int bishopPawnPenalty[9] { 0 };
    int bishopOppositePawnsPenalty[9][33]{ 0 };
    int rookBaseValue[33] { 0 };
    int rookPawnsPenalty[17] { 0 };

    // Helper functions
    void initTables();
    int evaluatePawns(int& value, const int& gameStage) const;
    int evaluateKnights(int& value, const int& gameStage) const;
    int evaluateBishops(int& value, const int& gameStage) const;
    int evaluateRooks(int& value, const int& gameStage) const;
    int evaluateQueens(int& value, const int& gameStage) const;

    enum Factors {PAWN_BASE_VALUE = 0, KNIGHT_BASE_VALUE, KNIGHT_PAWN_BONUS, KNIGHT_DISTANT_PAWNS_PENALTY,
        BISHOP_BASE_VALUE, BISHOP_PAIR_BONUS, BISHOP_COLOR_WEAKNESS_BONUS,
        ROOK_BASE_VALUE, ROOK_PAWNS_PENALTY,
        QUEEN_BASE_VALUE
    };
    static constexpr int factorsNum = 9;

public:
    MaterialBalance(BoardConfig* cnf, const FactorsVec& ftors) : PositionElement("MaterialBalance", ftors, factorsNum), config(cnf) { initTables(); }
    void setDependencies(PawnStructure* structure);
    // Static update
    void clearTables();
    void update() override;
    int evaluate(int& eval, const int& gameStage) const;
    // Dynamic update
    void updateByMove(int pieceID, const Square& oldPos, const Square& newPos);
    // Extra functionalities
    int stageValue() const {return totalPieceWeights[0] + totalPieceWeights[1];}
    int stageValueNormalised() const {return std::min(totalPieceWeights[0] + totalPieceWeights[1], maxStagePoints) / 8;}
    int countPawns(Side side) const {return pieceCount[0][side];}
    int countPawns() const {return pieceCount[0][0] + pieceCount[0][1];}
    int countKnights(Side side) const {return pieceCount[1][side];}
    int countBishops(Side side) const {return pieceCount[2][side];}
    bool hasBishopOfColor(Side side, SquareColors color) const { return bishops[color][side]; }
    bool hasBishopPair(Side side) const { return bishops[0][side] && bishops[1][side]; }
    int countRooks(Side side) const {return pieceCount[3][side];}
    int countQueens(Side side) const {return pieceCount[4][side];}
    int countHeavyPieces(Side side) const {return pieceCount[3][side] + pieceCount[4][side];}
    bool isInPawnsEndgame(Side side) const { return totalPieceWeights[side] < 9; }
    bool isPawnsEndgame() const { return totalPieceWeights[0] < 9 && totalPieceWeights[1] < 9; }
    bool isInKingAndMinorPieceEndgame(Side side) const { return totalPieceWeights[side] == 9; }
    bool isKingAndMinorPieceVsKingEndgame() const {return (totalPieceWeights[0] == 9 && totalPieceWeights[1] == 0) ||
                                                          (totalPieceWeights[0] == 0 && totalPieceWeights[1] == 9);}
    bool isOppositeBishopsEndgame() const {
        return totalPieceWeights[0] < 18 && totalPieceWeights[1] < 18 &&
           ((bishops[0][0] && bishops[1][1]) || (bishops[0][1] && bishops[1][0]));
    }
    bool isInClassicEndgame(Side side) const { return totalPieceWeights[side] < 42; }
    // Testing
    void show() const;
};

#endif // MATERIALBALANCE_H_INCLUDED
