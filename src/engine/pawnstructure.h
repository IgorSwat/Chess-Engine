#ifndef PAWNSTRUCTURE_H_INCLUDED
#define PAWNSTRUCTURE_H_INCLUDED

#include "positionelement.h"
#include "materialbalance.h"
#include "squarecontrol.h"

using std::vector;
typedef bool (*Comparator)(const int&, const int&);
typedef int (*DistanceCalculator)(const int&);

class PawnsWatcher;
class PassedPawns;

class PawnStructure : public PositionElement, public PositionChangedObserver
{
private:
    BoardConfig* config;
    MaterialBalance* material;
    SquareControl* control;

    // Data tables
    bool pawnsMap[8][8][2] { false };
    vector<int> pawnsAtColumns[8][2] {};

    int bottomPawns[8][2] { 0 };
    int upperPawns[8][2] { 0 };
    int pawnsAtColors[2][2] { 0 };
    int distantPawnsSum[2][2] { 0 };
    int pawnRowSums[2][2] { 0 };    // square color - side

    static constexpr int columnsDistanceWages[8] { 40, 13, 4, 1, 1, 4, 13, 40 };
    static constexpr int columnsSide[8] { 0, 0, 0, 0, 1, 1, 1, 1 };
    static constexpr int unitCondition = 3;
    static constexpr int sumCondition = 25;

    // Subelements
    vector<PawnsWatcher*> columnWatchers;
    vector<PawnsWatcher*> generalWatchers;
    PassedPawns* passed;

    // Evaluation elements
    int isolatedPawnColumnPoints[8] { 2, 3, 3, 4, 4, 3, 3, 2 };
    int isolatedPawnPointValue[33] { 0 };
    int doubledPawnValue[33] { 0 };
    int pawnIslandValue[33] { 0 };
    int backwardPawnValue[33] { 0 };
    int backwardPawnOpenFilePenalty = 0;
    int passedPawnBaseValue = 0;
    int passedPawnDistanceMultiplier[7] { 0 };
    int protectedPasserMultiplier = 1;
    int blockadeSinglePassersMultipliers[6] { 1, 2, 2, 2, 2, 2 };
    int blockadeConnectedPassersMultipliers[6] { 1, 1, 4, 1, 2, 2 };
    int passedPawnPointValue = 0;


    // Helper functions
    void initTables();
    void updateWatchersByInsertion(const vector<PawnsWatcher*>& watchers, const Side& side, const int& row, const int& col);
    void updateWatchersByRemoval(const vector<PawnsWatcher*>& watchers, const Side& side, const int& row, const int& col);
    void resetSubelements();

    void addPawn(const Side& side, const int& row, const int& col);
    void removePawn(const Side& side, const int& row, const int& col);

    enum Factors {ISOLATED_PAWNS = 0, DOUBLED_PAWNS, PAWN_ISLANDS, BACKWARD_PAWN_PENALTY, BACKWARD_PAWN_OPEN_FILE_PENALTY,
                  PASSED_PAWN_BASE_VALUE, PASSED_PAWN_DISTANCE_MULTIPLIER, PROTECTED_PASSER_MULTIPLIER, PASSED_PAWN_POINT_VALUE};

public:
    PawnStructure(BoardConfig* cnf, MaterialBalance* mt, SquareControl* ctr, const FactorsVec& ftors);
    ~PawnStructure();
    // Static update
    void clearTables();
    void reset();
    void update() override {reset();}
    int evaluate(int& eval, const int& gameStage) const override;
    // Dynamic update
    void updateByMove(int pieceID, const Square& oldPos, const Square& newPos) override;
    // Getters
    bool distantPawnsCheck(Side side) const;
    int countPawnsAtColumn(const Side& side, const int& col) const { return static_cast<int>(pawnsAtColumns[col][side].size()); }
    bool isPawnAtSquare(Side side, int col, int row) const {return pawnsMap[col][row][side]; }
    int bottomPawnFromColumn(Side side, int col) const {return bottomPawns[col][side]; }
    int pawnsAtColor(Side side, SquareColors color) const {return pawnsAtColors[color][side];}
    bool isFileSemiOpen(Side side, int col) const { return countPawnsAtColumn(side, col) == 0; }
    bool isFileOpen(int col) const { return countPawnsAtColumn(WHITE, col) == 0 && countPawnsAtColumn(BLACK, col) == 0; }
    int upperPawnRow(Side side, int col) const { return upperPawns[col][side]; }
    int getSumOfPawnRows(Side side, SquareColors color) const { return pawnRowSums[color][side]; }
    const vector<int>& getPawnRowsAtColumn(Side side, int col) { return pawnsAtColumns[col][side]; }
    // Testing
    void show() const;

    friend class IsolatedPawns;
    friend class DoubledPawns;
    friend class PawnIslands;
    friend class BackwardPawns;
    friend class PassedPawns;
};



class PawnsWatcher
{
public:
    virtual ~PawnsWatcher() = default;
    virtual void clearTables() = 0;
    virtual void updateByInsertion(Side side, int row, int col) = 0;
    virtual void updateByRemoval(Side side, int row, int col) = 0;
    virtual void evaluate(int& eval, const int& gameStage) const = 0;
    virtual void show() const = 0;
};



class IsolatedPawns : public PawnsWatcher
{
private:
    PawnStructure* positioning;
    int columnsIsolation[8][2] {0}; // 0-2 integer matching adjecent columns
    int isolatedPawns[2] { 0 };   // isolated pawns counters per side
public:
    IsolatedPawns(PawnStructure* pp) : positioning(pp) {}
    // Static update
    void clearTables() override;
    // Dynamic update
    void updateByInsertion(Side side, int row, int col)override ;
    void updateByRemoval(Side side, int row, int col) override;
    void evaluate(int& eval, const int& gameStage) const override;
    void show() const override { std::cout << "Isolated pawn points (white | black): " << isolatedPawns[0] << " | " << isolatedPawns[1] << std::endl; }
};



class DoubledPawns : public PawnsWatcher
{
private:
    PawnStructure* positioning;
    int doubledPawns[2] { 0 };
    // Helper functions
    int numberOfPairs(int pawnsCount) const {return pawnsCount * (pawnsCount - 1) / 2;}
public:
    DoubledPawns(PawnStructure* pp) : positioning(pp) {}
    // Static update
    void clearTables() override { doubledPawns[0] = doubledPawns[1] = 0; }
    // Dynamic update
    void updateByInsertion(Side side, int row, int col) override;
    void updateByRemoval(Side side, int row, int col) override;
    void evaluate(int& eval, const int& gameStage) const override;
    void show() const override { std::cout << "Doubled pawns (white | black): " << doubledPawns[0] << " | " << doubledPawns[1] << std::endl; }
};



class PawnIslands : public PawnsWatcher
{
private:
    PawnStructure* positioning;
    int pawnIslands[2] { 0 };
public:
    PawnIslands(PawnStructure* pp) : positioning(pp) {}
    // Static update
    void clearTables() override { pawnIslands[0] = pawnIslands[1] = 0; }
    // Dynamic update
    void updateByInsertion(Side side, int row, int col) override ;
    void updateByRemoval(Side side, int row, int col) override;
    void evaluate(int& eval, const int& gameStage) const override;
    void show() const override { std::cout << "Pawn islands (white | black): " << pawnIslands[0] << " | " << pawnIslands[1] << std::endl; }
};



class BackwardPawns : public PawnsWatcher
{
private:
    PawnStructure* positioning;
    // Data tables
    int adjecentFilesPropertyCount[8][2] { 0 };
    int pathControlledByEnemyPawn[8][8][2] { 0 };   // 0- 2
    int backwardPawnRows[8][2] { 0 };
    int backwardPawns[2] { 0 };
    int backwardPawnsAtSemiopenFiles[2] { 0 };

    static constexpr int endValues[2] { 0, 7 }; // There are no pawns existing on 0 or 7th row
    static constexpr int rowDiff[2] { -2, 2 };  // Knight's y-directions
    static constexpr int backwardLowerBounds[2] { 3, 1 };   // White pawns affect others in terms of backward check only on 3rd to 6th row (similiar for black)
    static constexpr int backwardUpperBounds[2] { 6, 4 };   //
    static constexpr int excludedRows[2] { 3, 4 };  // When entering the opponent's half of the board pawn can't become backward

    void updateAdjecentFilesCounter(Side side, int col);
    void updateOpponentPawnsOnAdjecentFileWhenAddingPawn(Side side, Side opposite, int col, int oppositeRow);
    void updateOwnPawnsOnAdjecentFileWhenAddingPawn(Side side, Side opposite, int col);
    void updateOpponentPawnsOnAdjecentFileWhenRemovingPawn(Side side, Side opposite, int col, int oppositeRow);
    void updateOwnPawnsOnAdjecentFileWhenRemovingPawn(Side side, Side opposite, int col);

    void updateBackwardCounters(Side side, Side opposite, int col, int diff) {
        backwardPawns[side] += diff;
        if (positioning->isFileSemiOpen(opposite, col))
            backwardPawnsAtSemiopenFiles[side] += diff;
    }

public:
    BackwardPawns(PawnStructure* pp) : positioning(pp) {}
    // Static update
    void clearTables() override;
    void evaluate(int& eval, const int& gameStage) const override;
    // Dynamic update
    void updateByInsertion(Side side, int row, int col) override;
    void updateByRemoval(Side side, int row, int col) override;
    void show() const override;
};



class PassedPawns : public PawnsWatcher
{
private:
    PawnStructure* positioning;
    BoardConfig* config;
    SquareControl* control;
    int passedConditionCounters[8][2] { 0 };    // 0 - 3
    int passedPawnRows[8][2] { 0 };
    int passersPoints[8][2][3] { 0 };
    int passedPawnCounters[2] { 0 };
    int passedPawnPoints[2] { 0 };

    static int calculateConnectedPawnsPoints(int p1, int p2) { return p1 * p2 / 4; }
    void addPassedPawn(Side side, int row, int col);
    void removePassedPawn(Side side, int row, int col);
    void updateCountersWhenAddingPawn(Side side, Side opposite, int targetCol, int pawnRow, int pawnCol);
    void updateCountersWhenRemovingPawn(Side side, Side opposite, int targetCol, int pawnRow, int pawnCol);
    int getPointsAfterPenalties(Side side, int row, int col);

    enum ColumnSide {LEFT_SIDE = 0, SELF_POINTS = 1, RIGHT_SIDE = 2};
    enum ParametrizationMode { MULTIPLICATE = 0, DIVIDE = 1 };

    void updateSelfPoints(Side side, int row, int col, int factor, ParametrizationMode mode);
    void updatePairPoints(Side side, int row, int col, int factor, ParametrizationMode mode);
public:
    PassedPawns(PawnStructure* pp, BoardConfig* cnf, SquareControl* ctr) : positioning(pp), config(cnf), control(ctr) {}
    void clearTables() override;
    void evaluate(int& eval, const int& gameStage) const override;
    // Dynamic update
    void updateByInsertion(Side side, int row, int col) override;
    void updateByRemoval(Side side, int row, int col) override;
    void updateByPiecePositionChanged(Side side, PieceType type, int row, int col, ParametrizationMode mode);
    void show() const override;

    friend class PawnStructure;
};


#endif // PAWNSTRUCTURE_H_INCLUDED
