#ifndef PAWNSTRUCTURE_H_INCLUDED
#define PAWNSTRUCTURE_H_INCLUDED

#include "positionelement.h"
#include "materialbalance.h"
using std::vector;

class PawnsWatcher;
class PawnsWatcherExtended;

class PawnStructure : public PositionElement, public PositionChangedObserver, public PieceTypeChangedObserver
{
private:
    BoardConfig* config;
    MaterialBalance* material;
    /// Data tables
    bool pawnsMap[8][8][2];
    int tableBottomPawns[8][2];
    int tablePawnsAtColumn[8][2];
    int topLeftPawns[2];
    int topRightPawns[2];
    int tablePawnsAtColor[2][2];
    int tableSemiOpenFiles[2];
    /// Subelements
    vector<PawnsWatcher*> pawnsWatchers;
    vector<PawnsWatcherExtended*> pawnsWatchersAdvanced;
    /// Helper functions
    void resetSubelements();
    void addPawn(int col, int row, int colorID, bool columnUpdateFlag = true);
    void removePawn(int col, int row, int colID, bool columnUpdateFlag = true);
public:
    PawnStructure(BoardConfig* cnf, MaterialBalance* mt);
    ~PawnStructure();
    /// Static update
    void clearTables();
    void reset() override;
    void evaluate() override {reset();}
    /// Dynamic update
    void updateByMove(int pieceID, const sf::Vector2i& oldPos, const sf::Vector2i& newPos);
    void updateByChange(int pieceID, const sf::Vector2i& oldPos, PieceType oldType, PieceType newType);
    void updateElementsByInsertion(int column, COLOR color);
    void updateElementsByInsertion(int column, int row, COLOR color);
    void updateElementsByRemoval(int column, COLOR color);
    void updateElementsByRemoval(int column, int row, COLOR color);
    /// Getters
    int pawnsAt(int column, int colorID) {return tablePawnsAtColumn[column][colorID];}
    bool isPawnAt(int col, int row, int colorID) {return pawnsMap[col][row][colorID];}
    int bottomPawnFromColumn(int column, int sideID) {return tableBottomPawns[column][sideID];}
    int maxPawnsDistance(COLOR side) {return topRightPawns[(int)side] - topLeftPawns[(int)side];}
    int pawnsAtColor(COLOR side, int squareColor) {return tablePawnsAtColor[squareColor][(int)side];}
    int semiOpenFiles(COLOR side) const {return tableSemiOpenFiles[(int)side];}
    /// Testing
    void show() const;
};



class PawnsWatcher : public PositionElement
{
public:
    PawnsWatcher(const std::string& elementName) : PositionElement(elementName) {}
    virtual ~PawnsWatcher() {}
    virtual void clearTables() = 0;
    void reset() {clearTables();}
    virtual void updateByInsertion(int column, COLOR color) = 0;
    virtual void updateByRemoval(int column, COLOR color) = 0;
};

class PawnsWatcherExtended : public PositionElement
{
public:
    PawnsWatcherExtended(const std::string& elementName) : PositionElement(elementName) {}
    virtual ~PawnsWatcherExtended() {}
    virtual void clearTables() = 0;
    void reset() {clearTables();}
    virtual void updateByInsertion(int column, int row, COLOR color) = 0;
    virtual void updateByRemoval(int column, int row, COLOR color) = 0;
};



class IsolatedPawns : public PawnsWatcher
{
private:
    PawnStructure* positioning;
    int value;
    int columnsIsolation[8][2];
public:
    IsolatedPawns(PawnStructure* pp) : PawnsWatcher("IsolatedPawns"), positioning(pp) {}
    /// Static update
    void clearTables();
    void evaluate() override {positioning->reset();}
    /// Dynamic update
    void updateByInsertion(int column, COLOR color);
    void updateByRemoval(int column, COLOR color);
    void show() const {std::cout<<"Isolated pawns difference: "<<value<<std::endl;}
};



class DoubledPawns : public PawnsWatcher
{
private:
    PawnStructure* positioning;
    int value;
    /// Helper functions
    int numberOfPair(int pawnsCount) const {return pawnsCount * (pawnsCount - 1) / 2;}
public:
    DoubledPawns(PawnStructure* pp) : PawnsWatcher("DoubledPawns"), positioning(pp) {}
    /// Static update
    void clearTables() {value = 0;}
    void evaluate() override {positioning->reset();}
    /// Dynamic update
    void updateByInsertion(int column, COLOR color);
    void updateByRemoval(int column, COLOR color);
    void show() const {std::cout<<"Doubled pawns difference: "<<value<<std::endl;}
};



class PawnIslands : public PawnsWatcher
{
private:
    PawnStructure* positioning;
    int value;
public:
    PawnIslands(PawnStructure* pp) : PawnsWatcher("PawnIslands"), positioning(pp) {}
    /// Static update
    void clearTables() {value = 0;}
    void evaluate() override {positioning->reset();}
    /// Dynamic update
    void updateByInsertion(int column, COLOR color);
    void updateByRemoval(int column, COLOR color);
    void show() const {std::cout<<"Pawn islands difference: "<<value<<std::endl;}
};



class BackwardPawns : public PawnsWatcherExtended
{
private:
    PawnStructure* positioning;
    /// Data tables
    int value;
    short sentinels[8][8][2]; // Sentinels for white means black pawns
    bool bottomPawnFlags[8][8][2];
    /// Helper functions
    bool isBackwards(int col, int row, int sideID) {return bottomPawnFlags[row][col][sideID] && sentinels[row][col][sideID] > 0;}
public:
    BackwardPawns(PawnStructure* pp) : PawnsWatcherExtended("BackwardPawns"), positioning(pp) {}
    /// Static update
    void clearTables();
    void evaluate() override {positioning->reset();}
    /// Dynamic update
    void updateByInsertion(int column, int row, COLOR color);
    void updateByRemoval(int column, int row, COLOR color);
    void show() const {std::cout<<"Backward pawns difference: "<<value<<std::endl;}
};

#endif // PAWNSTRUCTURE_H_INCLUDED
