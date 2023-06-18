#ifndef SQUARECONTROL_H_INCLUDED
#define SQUARECONTROL_H_INCLUDED

#include "positionelement.h"
#include "misc.h"
using std::vector;

class SquareControl : public PositionElement, public ObservablePawnControl,
    public PositionChangedObserver, public MoveListChangedObserver
{
private:
    BoardConfig* config;
    MoveGenerator* generator;
    // Data tables
    int control[8][8][2] {};
    int pawnControl[8][8][2] {};
    int* counterPointers[8][8][2] {};
    int ownCampControl[2] {};
    int opponentCampControl[2] {};
    int centerControl[2] {};
    static constexpr int pieceCodes[6] { 4984, 408, 408, 40, 4, 2 };
    // Helper functions
    void initPointers();
    void updateControlTables(const sf::Vector2i& pos, const int& prevState, const int& currState);
    int whoControls(int col, int row) const {
        int controlDiff = control[row][col][0] - control[row][col][1];
        if (controlDiff > 0) return 0; else return controlDiff < 0 ? 1 : -1;
    }
    int whoControls(const sf::Vector2i& pos) const {return whoControls(pos.x, pos.y);}
    bool isCentralSquare(const sf::Vector2i& pos) const {return (pos.x == 3 || pos.x == 4) && (pos.y == 3 || pos.y == 4);}
    bool isSquareDefended(const sf::Vector2i& square, COLOR side) { return control[square.y][square.x][(int)side] > 1; }
    int attackersToSquare(const sf::Vector2i& square, COLOR side, PieceType type) { return control[square.y][square.x][(int)side] / pieceCodes[type]; }
public:
    SquareControl(MoveGenerator* gen, BoardConfig* cnf);
    ~SquareControl() {}
    // Static update
    void clearTables();
    void evaluate();
    void reset() override {evaluate();}
    // Dynamic update
    void updateByMove(int pieceID, const sf::Vector2i& oldPos, const sf::Vector2i& newPos) override;
    void updateByInsertion(const Move2& move) override;
    void updateByRemoval(const vector<Move2>& moves) override;
    // Extra functionalities
    bool isControlledByPawn(int col, int row, int side) {return pawnControl[row][col][side] > 0;}
    bool isControlledByPawn(const sf::Vector2i& pos, int side) {return pawnControl[pos.y][pos.x][side] > 0;}
    int ownCampControlDifference() const {return ownCampControl[0] - ownCampControl[1];}
    int opponentCampControlDifference() const {return opponentCampControl[0] - opponentCampControl[1];}
    int centerControlDifference() const {return centerControl[0] - centerControl[1];}
    // Testing
    void show() const;
};

#endif // SQUARECONTROL_H_INCLUDED
