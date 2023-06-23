#ifndef SQUARECONTROL_H_INCLUDED
#define SQUARECONTROL_H_INCLUDED

#include "positionelement.h"
using std::vector;

enum ThreatType { HANGING_PIECE = 0, LOWER_TYPE_ATTACK };

class Connectivity;

class SquareControl : public PositionElement, public ObservablePawnControl,
    public PositionChangedObserver, public MoveListChangedObserver
{
private:
    BoardConfig* config;
    MoveGenerator* generator;
    Connectivity* connectivity;
    // Data tables
    int control[8][8][2] {};
    int pawnControl[8][8][2] {};
    int* counterPointers[8][8][2] {};
    int ownCampControl[2] {};
    int opponentCampControl[2] {};
    int centerControl[2] {};
    int attackers[5]{ };
    static constexpr int pieceCodes[6] { 4984, 408, 408, 40, 4, 2 };
    // Helper functions
    template <typename Container> void updateFromRemovalConstruct(int pieceID, Container moves);
    void initPointers();
    void updateControlTables(const sf::Vector2i& pos, const int& prevState, const int& currState);
    int whoControls(int col, int row) const {
        int controlDiff = control[row][col][0] - control[row][col][1];
        if (controlDiff > 0) return 0; else return controlDiff < 0 ? 1 : -1;
    }
    int whoControls(const sf::Vector2i& pos) const {return whoControls(pos.x, pos.y);}
    bool isCentralSquare(const sf::Vector2i& pos) const {return (pos.x == 3 || pos.x == 4) && (pos.y == 3 || pos.y == 4);}
public:
    SquareControl(MoveGenerator* gen, BoardConfig* cnf);
    ~SquareControl();
    // Static update
    void clearTables();
    void evaluate();
    void reset() override {evaluate();}
    // Dynamic update
    void updateByMove(int pieceID, const sf::Vector2i& oldPos, const sf::Vector2i& newPos) override;
    void updateByInsertion(const Move2& move);
    void updateByRemoval(int pieceID, const vector<Move2>& moves) { updateFromRemovalConstruct(pieceID, moves); }
    void updateByRemoval(int pieceID, const MoveList& moves) { updateFromRemovalConstruct(pieceID, moves); }
    // Extra functionalities
    bool isSquareDefended(const sf::Vector2i& square, COLOR side) const { return control[square.y][square.x][(int)side] > 1; }
    const int* countAttackers(const sf::Vector2i& square, COLOR side);
    bool isControlledByPawn(int col, int row, int side) {return pawnControl[row][col][side] > 0;}
    bool isControlledByPawn(const sf::Vector2i& pos, int side) {return pawnControl[pos.y][pos.x][side] > 0;}
    int ownCampControlDifference() const {return ownCampControl[0] - ownCampControl[1];}
    int opponentCampControlDifference() const {return opponentCampControl[0] - opponentCampControl[1];}
    int centerControlDifference() const {return centerControl[0] - centerControl[1];}
    // Testing
    void show() const;
    friend class Connectivity;
};



class Connectivity
{
private:
    BoardConfig* config;
    SquareControl* control;
    int threats[8][8][2]{ 0 };	// (square.y - square.x - threat_type)
    int totalThreats[2][2][4]{ 0 };	// (side - threat_type - piece_type)
    bool isHanging(const sf::Vector2i& pos) { return threats[pos.y][pos.x][HANGING_PIECE] > 0 && threats[pos.y][pos.x][LOWER_TYPE_ATTACK] == 0; }
public:
    Connectivity(BoardConfig* cnf, SquareControl* cnt) : config(cnf), control(cnt) {}
    void clearTables();
    // Dynamic update (trigerred by SquareControl)
    void updateByMove(int pieceID, const sf::Vector2i& oldPos, const sf::Vector2i& newPos);
    void updateByInsertion(const Move2& move, const bool& prevDefenseState);
    void updateByRemoval(const Move2& move, const COLOR& side, const COLOR& opposite);
    void showThreats(COLOR side) const;
    void show() const;
    friend class SquareControl;
};



template <typename Container>
void SquareControl::updateFromRemovalConstruct(int pieceID, Container moves)
{
    COLOR side = pieceID < 16 ? COLOR::WHITE : COLOR::BLACK;
    COLOR opposite = opposition(side);
    for (const Move2& move : moves)
    {
        if (move.specialFlag.isAttacking())
        {
            int prevState = whoControls(move.targetPos);
            control[move.targetPos.y][move.targetPos.x][(int)side] -= pieceCodes[(int)move.pieceType];
            int currState = whoControls(move.targetPos);
            if (currState != prevState)
                updateControlTables(move.targetPos, prevState, currState);
            if (move.pieceType == PieceType::PAWN)
            {
                pawnControl[move.targetPos.y][move.targetPos.x][(int)side] -= 1;
                if (pawnControl[move.targetPos.y][move.targetPos.x][(int)side] == 0)
                    updateObserversByDisappearance(move.targetPos, (int)side);
            }
            config->removeAttack(move.targetPos, (int)side);
            connectivity->updateByRemoval(move, side, opposite);
        }
    }
}




#endif // SQUARECONTROL_H_INCLUDED
