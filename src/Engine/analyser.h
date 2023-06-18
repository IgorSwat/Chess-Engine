#ifndef ANALYSER_H
#define ANALYSER_H

#include "positionelement.h"
#include "materialbalance.h"
#include "pawnstructure.h"
#include "piecemobility.h"
using std::vector;


enum FactorsOrder {PAWN_BASE_VALUE = 0,
        KNIGHT_BASE_VALUE = 1, KNIGHT_PAWN_BONUS, KNIGHT_DISTANT_PAWNS_PENALTY, KNIGHT_VS_ROOK_PENALTY,
        BISHOP_BASE_VALUE = 5, BISHOP_PAIR_BONUS, BISHOP_OWN_PAWNS_PENALTY, BISHOP_OPPONENT_PAWNS_PENALTY,
        ROOK_BASE_VALUE = 9, ROOK_OPEN_FILES_BONUS, ROOK_PAIR_PENALTY,
        QUEEN_BASE_VALUE = 12,
        OWN_CAMP_CONTROL = 13, OPPONENT_CAMP_CONTROL, CENTER_CONTROL,
        KNIGHT_MOBILITY_OPENING = 16, KNIGHT_MOBILITY_ENDGAME, BISHOP_MOBILITY_OPENING, BISHOP_MOBILITY_ENDGAME,
        ROOK_MOBILITY_OPENING, ROOK_MOBILITY_ENDGAME, QUEEN_MOBILITY_OPENING, QUEEN_MOBILITY_ENDGAME,
        PROTECTION_POINTS = 24};
enum ElementsOrder {MATERIAL_BALANCE = 0, PAWN_STRUCTURE = 1, SQUARE_CONTROL, MOBILITY, CONNECTIVITY};
enum SquareColors {LIGHT_SQUARE = 0, DARK_SQUARE};


class Factor
{
public:
    Factor(short beg, short end) : startValue(beg), endValue(end) {}
    short startValue;
    short endValue;
};


class Analyser
{
private:
    BoardConfig* config;
    MoveGenerator* generator;
    MaterialBalance* material;
    PawnStructure* structure;
    SquareControl* control;
    PieceMobility* mobility;
    vector<PositionElement*> elements;
    vector<Factor> factors;
    // Factor tables
    int pawnBaseValue[33];
    int knightPawnBonus[17];
    int knightDistantPawnsPenalty[33];
    int knightsForRookPenalty[33];
    int bishopPawnPenalty[9];
    int bishopOppositePawnsPenalty[9][33];
    int rookBaseValue[33];
    int rookOpenFilesBonus[9];
    int ownCampControlValue[33];
    int oppositeCampControlValue[33];
    int centerControlValue[33];
    int knightMobility[9][33];
    int bishopMobility[14][33];
    int rookMobility[15][33];
    int queenMobility[28][33];
    // Initialisers
    void initElements();
    void initFactors();
    void initTables();
    // Interpolation functions
    int gameStageInterpolation(const Factor& vals) const;
    int gameStageInterpolation(const int& begVal, const int& endVal) const;
    int interpolation(const int& begVal, const int& endVal, const float& w, float (*func)(float));
    int interpolation(const Factor& vals, const float& w, float (*func)(float)) {return interpolation(vals.startValue, vals.endValue,
                                                                                                      w, func);}
    static float mobilityProgressFunction(float x);
public:
    Analyser(BoardConfig* conf, MoveGenerator* generator);
    ~Analyser();
    void evaluateAll();
    // Analysing elements
    int evaluatePawns(int& value, const int& gameStage) const;
    int evaluateKnights(int& value, const int& gameStage) const;
    int evaluateBishops(int& value, const int& gameStage) const;
    int evaluateRooks(int& value, const int& gameStage) const;
    int evaluateQueens(int& value, const int& gameStage) const;
    int evaluateSquareControl(int& value, const int& gameStage) const;
    int evaluateMobility(int& value, const int& gameStage) const;
    int evaluatePosition() const;
    void show() const;
};

#endif // ANALYSER_H
