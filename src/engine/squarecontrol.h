#pragma once

#include "positionelement.h"
using std::vector;


class Connectivity;

class SquareControl : public PositionElement, public ObservablePawnControl,
    public PositionChangedObserver, public MoveListChangedObserver
{
private:
    BoardConfig* config;
    MoveGenerator* generator;
    // Data tables
    int control[8][8][2] { 0 };
    int* counterPointers[8][8][2] { nullptr };
    int ownCampControl[2] { 0 };
    int opponentCampControl[2] { 0 };
    int centerControl[2] { 0 };
    int attackers[5]{ 0 };
    static constexpr int pieceCodes[6] { 4984, 408, 408, 40, 4, 2 };
    // Evaluation elements
    static constexpr int threatsFactor = 10;
    static constexpr int threatsValues[4]{ 3, 5, 7, 10 };
    int ownCampControlValue[33]{ 0 };
    int oppositeCampControlValue[33]{ 0 };
    int centerControlValue[33]{ 0 };
    // Helper functions
    template <typename Container> void updateFromRemovalConstruct(int pieceID, Container moves);
    void initPointers();
    void initTables();
    void updateControlTables(const Square& pos, const int& prevState, const int& currState);
    int whoControls(const int& col, const int& row) const {
        int controlDiff = control[row][col][0] - control[row][col][1];
        if (controlDiff > 0) return 0; else return controlDiff < 0 ? 1 : -1;
    }
    int whoControls(const Square& pos) const {return whoControls(pos.x, pos.y);}
    int evaluateSquareControl(int& eval, const int& gameStage) const;

    enum Factors { OWN_CAMP_CONTROL = 0, OPPONENT_CAMP_CONTROL, CENTER_CONTROL };

public:
    SquareControl(MoveGenerator* gen, BoardConfig* cnf, const FactorsVec& ftors);
    // Static update
    void clearTables();
    void update() override;
    int evaluate(int& eval, const int& gameStage) const override;
    // Dynamic update
    void updateByMove(int pieceID, const Square& oldPos, const Square& newPos) override;
    void updateByInsertion(const Move& move);
    void updateByRemoval(int pieceID, const vector<Move>& moves, bool legal) override { updateFromRemovalConstruct(pieceID, moves); }
    void updateByRemoval(int pieceID, const MoveList& moves, bool legal) override { updateFromRemovalConstruct(pieceID, moves); }
    // Extra functionalities
    int countPawnAttacks(Side side, int row, int col) { return static_cast<int>(control[row][col][side] / pieceCodes[0]); }
    int countPawnAttacks(const Square& square, Side side) const { return static_cast<int>(control[square.y][square.x][side] / pieceCodes[0]); }
    bool isSquareDefended(const Square& square, Side side) const { return control[square.y][square.x][side] > 1; }
    const int* countAttackers(const Square& square, Side side, PieceType typeBoundary);
    bool isControlledByPawn(const Square& pos, Side side) {return countPawnAttacks(pos, side) > 0; }
    int ownCampControlDifference() const {return ownCampControl[0] - ownCampControl[1];}
    int opponentCampControlDifference() const {return opponentCampControl[0] - opponentCampControl[1];}
    int centerControlDifference() const {return centerControl[0] - centerControl[1];}
    // Testing
    void show() const;
};


template <typename Container>
void SquareControl::updateFromRemovalConstruct(int pieceID, Container moves)
{
    Side side = pieceID < 16 ? WHITE : BLACK;
    Side opposite = opposition[side];
    for (const Move& move : moves)
    {
        if (move.hasProperty(Moves::ATTACK_FLAG) && (move.promotionType == PAWN || move.promotionType == QUEEN))
        {
            int prevState = whoControls(move.targetPos);
            control[move.targetPos.y][move.targetPos.x][side] -= pieceCodes[(int)move.pieceType];
            int currState = whoControls(move.targetPos);
            if (currState != prevState)
                updateControlTables(move.targetPos, prevState, currState);
            if (move.pieceType == PieceType::PAWN && countPawnAttacks(move.targetPos, side) == 0)
                updateObserversByDisappearance(move.targetPos, side);
        }
    }
}