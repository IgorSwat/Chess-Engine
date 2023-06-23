#include "analyser.h"
#include <cmath>
using std::string;

Analyser::Analyser(BoardConfig* conf, MoveGenerator* gen)
{
    config = conf;
    generator = gen;
    initFactors();
    initElements();
}

void Analyser::initElements()
{
    material = new MaterialBalance(config);
    config->addObserver(material);
    elements.push_back(material);
    structure = new PawnStructure(config, material);
    config->addObserver(structure);
    material->addObserver(structure);
    elements.push_back(structure);
    control = new SquareControl(generator, config);
    generator->addObserver(control);
    config->addObserver(control);
    elements.push_back(control);
    mobility = new PieceMobility(generator, control);
    generator->addObserver(mobility);
    control->addObserver(mobility);
    elements.push_back(mobility);
}

void Analyser::initFactors()
{
    // wages in order:  0% - 100%
    // but, game stage wages are kinda reversed:  opening - endgame

    factors.push_back(Factor(100, 125));    // Pawn base value (linear, stage)
    factors.push_back(Factor(350, 350));    // Knight base value
    factors.push_back(Factor(0, 50));       // Knight bonus for each pawn on the board  (quadratic)
    factors.push_back(Factor(-5, -40));     // Knight penalty for opposite distant pawns (linear, stage)
    factors.push_back(Factor(30, -72));      // Knights penalty for 2 knights vs 1 rook imbalance (linear, stage)
    factors.push_back(Factor(375, 375));    // Bishop base value
    factors.push_back(Factor(50, 50));      // Bishop pair bonus
    factors.push_back(Factor(0, -50));      // Bishop penalty for pawns on squares of the same color as bishop (quadratic)
    factors.push_back(Factor(45, -25));     // Bishop penalty / bonus for opposite pawns on squares of the same color as bishop (quadratic - linear, stage)
    factors.push_back(Factor(500, 600));    // Rook base value (linear, stage)
    factors.push_back(Factor(0, 30));       // Rook semi-open files bonus (square root)  (to improve!)
    factors.push_back(Factor(-30, -30));     // Rook pair penalty
    factors.push_back(Factor(1150, 1150));  // Queen base value
    factors.push_back(Factor(5, 3));        // Own squares control value   (linear, stage)
    factors.push_back(Factor(8, 6));      // Opponent`s squares control value    (linear, stage)
    factors.push_back(Factor(12, 9));      // Central squares control value   (linear stage)
    factors.push_back(Factor(-46, 42));     // Knight mobility in opening (mobility function)
    factors.push_back(Factor(-50, 30));     // Knight mobility in endgame (mobility function)
    factors.push_back(Factor(-32, 76));     // Bishop mobility in opening (mobility function)
    factors.push_back(Factor(-50, 85));     // Bishop mobility in endgame (mobility function)
    factors.push_back(Factor(-35, 54));     // Rook mobility in opening (mobility function)
    factors.push_back(Factor(-69, 132));     // Rook mobility in endgame (mobility function)
    factors.push_back(Factor(-16, 72));     // Queen mobility in opening (mobility function)
    factors.push_back(Factor(-35, 188));     // Queen mobility in endgame (mobility function)
    factors.push_back(Factor(7, 1));         // Protection points value (7 protection points = 1 centipawn)
    initTables();
}

void Analyser::initTables()
{
    /// Only game stage dependable
    for (int i = 0; i < 33; i++)
    {
        float w = (32.f - i) / 32.f;
        pawnBaseValue[i] = interpolation(factors[PAWN_BASE_VALUE], w, [](float x)->float{return x;});
        knightDistantPawnsPenalty[i] = interpolation(factors[KNIGHT_DISTANT_PAWNS_PENALTY], w, [](float x)->float{return x;});
        knightsForRookPenalty[i] = interpolation(factors[KNIGHT_VS_ROOK_PENALTY], w, [](float x)->float{return x;});
        rookBaseValue[i] = interpolation(factors[ROOK_BASE_VALUE], w, [](float x)->float{return x;});
        ownCampControlValue[i] = interpolation(factors[OWN_CAMP_CONTROL], w, [](float x)->float{return x;});
        oppositeCampControlValue[i] = interpolation(factors[OPPONENT_CAMP_CONTROL], w, [](float x)->float{return x;});
        centerControlValue[i] = interpolation(factors[CENTER_CONTROL], w, [](float x)->float{return x;});
    }
    /// Others
    for (int i = 0; i < 17; i++)
        knightPawnBonus[i] = interpolation(factors[KNIGHT_PAWN_BONUS], i / 16.f, [](float x)->float{return x * x;});
    for (int i = 0; i < 9; i++)
        bishopPawnPenalty[i] = interpolation(factors[BISHOP_OWN_PAWNS_PENALTY], i / 8.f, [](float x)->float{return x * x;});
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 33; j++)
        {
            int interpolationValue = interpolation(factors[BISHOP_OPPONENT_PAWNS_PENALTY], i / 8.f, [](float x)->float{return x * x;});
            bishopOppositePawnsPenalty[i][j] = interpolation(interpolationValue, -interpolationValue,
                                                    (32.f - j) / 32.f, [](float x)->float{return x;});
        }
    }
    for (int i = 0; i < 9; i++)
        rookOpenFilesBonus[i] = interpolation(factors[ROOK_OPEN_FILES_BONUS], i / 8.f, [](float x)->float{return std::sqrt(x);});
    for (int i = 0; i < 9; i++)
    {
        int opening = interpolation(factors[KNIGHT_MOBILITY_OPENING], i / 8.f, mobilityProgressFunction);
        int endgame = interpolation(factors[KNIGHT_MOBILITY_ENDGAME], i / 8.f, mobilityProgressFunction);
        for (int j = 0; j < 33; j++)
            knightMobility[i][j] = interpolation(opening, endgame, (32.f - j) / 32.f, [](float x)->float{return x;});
    }
    for (int i = 0; i < 14; i++)
    {
        int opening = interpolation(factors[BISHOP_MOBILITY_OPENING], i / 13.f, mobilityProgressFunction);
        int endgame = interpolation(factors[BISHOP_MOBILITY_ENDGAME], i / 13.f, mobilityProgressFunction);
        for (int j = 0; j < 33; j++)
            bishopMobility[i][j] = interpolation(opening, endgame, (32.f - j) / 32.f, [](float x)->float{return x;});
    }
    for (int i = 0; i < 15; i++)
    {
        int opening = interpolation(factors[ROOK_MOBILITY_OPENING], i / 14.f, mobilityProgressFunction);
        int endgame = interpolation(factors[ROOK_MOBILITY_ENDGAME], i / 14.f, mobilityProgressFunction);
        for (int j = 0; j < 33; j++)
            rookMobility[i][j] = interpolation(opening, endgame, (32.f - j) / 32.f, [](float x)->float{return x;});
    }
    for (int i = 0; i < 28; i++)
    {
        int opening = interpolation(factors[QUEEN_MOBILITY_OPENING], i / 27.f, mobilityProgressFunction);
        int endgame = interpolation(factors[QUEEN_MOBILITY_ENDGAME], i / 27.f, mobilityProgressFunction);
        for (int j = 0; j < 33; j++)
            queenMobility[i][j] = interpolation(opening, endgame, (32.f - j) / 32.f, [](float x)->float{return x;});
    }
}

Analyser::~Analyser()
{
    for (PositionElement* element : elements)
        delete element;
}




inline int Analyser::gameStageInterpolation(const Factor& vals) const
{
    short stageValue = material->stageValue();
    return vals.startValue * stageValue / MaterialBalance::maxStagePoints +
        vals.endValue * (MaterialBalance::maxStagePoints - stageValue) / MaterialBalance::maxStagePoints;
}

inline int Analyser::gameStageInterpolation(const int& begVal, const int& endVal) const
{
    short stageValue = material->stageValue();
    return begVal * stageValue / MaterialBalance::maxStagePoints +
        endVal * (MaterialBalance::maxStagePoints - stageValue) / MaterialBalance::maxStagePoints;
}

inline int Analyser::interpolation(const int& begVal, const int& endVal, const float& w, float (*func)(float))
{
    return static_cast<short>(begVal * func(1.f - w) + endVal * func(w));
}

float Analyser::mobilityProgressFunction(float x)
{
    if (x < 1.f / 3.f)
        return 500.f * sqrt(x) / 577.f;
    else if (x >= 1.f / 3.f && x <= 2.f / 3.f)
        return 1.f / 2.f + 3.f * (x - 1.f / 3.f) / 5.f;
    else
        return 9.f * x * x / 10.f - 3.f * x / 5.f + 7.f / 10.f;
}



int Analyser::evaluatePawns(int& value, const int& gameStage) const
{
    short startValue = value;
    value += (material->countPawns(COLOR::WHITE) - material->countPawns(COLOR::BLACK)) * pawnBaseValue[gameStage];
    return value - startValue;
}

int Analyser::evaluateKnights(int& value, const int& gameStage) const
{
    short startValue = value;
    short whiteKnights = material->countKnights(COLOR::WHITE);
    short blackKnights = material->countKnights(COLOR::BLACK);
    value += (whiteKnights - blackKnights) * factors[KNIGHT_BASE_VALUE].startValue;
    value += (whiteKnights - blackKnights) * knightPawnBonus[material->countPawns()];
    if (structure->maxPawnsDistance(COLOR::BLACK) > 4)
        value += whiteKnights * knightDistantPawnsPenalty[gameStage];
    if (structure->maxPawnsDistance(COLOR::WHITE) > 4)
        value -= blackKnights * knightDistantPawnsPenalty[gameStage];
    short rooksDiff = material->countRooks(COLOR::WHITE) - material->countRooks(COLOR::BLACK);
    if (whiteKnights - blackKnights >= 2 && rooksDiff < 0)
        value += knightsForRookPenalty[gameStage];
    else if (whiteKnights - blackKnights <= -2 && rooksDiff > 0)
        value -= knightsForRookPenalty[gameStage];
    return value - startValue;
}

int Analyser::evaluateBishops(int& value, const int& gameStage) const
{
    short startValue = value;
    value += (material->countBishops(COLOR::WHITE) - material->countBishops(COLOR::BLACK)) * factors[BISHOP_BASE_VALUE].startValue;
    if (material->hasBishopOfColor(COLOR::WHITE, LIGHT_SQUARE))
    {
        if (material->hasBishopOfColor(COLOR::WHITE, DARK_SQUARE))
            value += factors[BISHOP_PAIR_BONUS].startValue;
        value += bishopPawnPenalty[structure->pawnsAtColor(COLOR::WHITE, LIGHT_SQUARE)];
        value += bishopOppositePawnsPenalty[structure->pawnsAtColor(COLOR::BLACK, LIGHT_SQUARE)][gameStage];
    }
    if (material->hasBishopOfColor(COLOR::WHITE, DARK_SQUARE))
    {
        value += bishopPawnPenalty[structure->pawnsAtColor(COLOR::WHITE, DARK_SQUARE)];
        value += bishopOppositePawnsPenalty[structure->pawnsAtColor(COLOR::BLACK, DARK_SQUARE)][gameStage];
    }
    if (material->hasBishopOfColor(COLOR::BLACK, LIGHT_SQUARE))
    {
        if (material->hasBishopOfColor(COLOR::BLACK, DARK_SQUARE))
            value -= factors[BISHOP_PAIR_BONUS].startValue;
        value -= bishopPawnPenalty[structure->pawnsAtColor(COLOR::BLACK, LIGHT_SQUARE)];
        value -= bishopOppositePawnsPenalty[structure->pawnsAtColor(COLOR::WHITE, LIGHT_SQUARE)][gameStage];
    }
    if (material->hasBishopOfColor(COLOR::BLACK, DARK_SQUARE))
    {
        value -= bishopPawnPenalty[structure->pawnsAtColor(COLOR::BLACK, DARK_SQUARE)];
        value -= bishopOppositePawnsPenalty[structure->pawnsAtColor(COLOR::WHITE, DARK_SQUARE)][gameStage];
    }
    return value - startValue;
}

int Analyser::evaluateRooks(int& value, const int& gameStage) const
{
    short startValue = value;
    short whiteRooks = material->countRooks(COLOR::WHITE);
    short blackRooks = material->countRooks(COLOR::BLACK);
    value += (whiteRooks - blackRooks) * rookBaseValue[gameStage];
    value += whiteRooks * rookOpenFilesBonus[structure->semiOpenFiles(COLOR::WHITE)];
    value -= blackRooks * rookOpenFilesBonus[structure->semiOpenFiles(COLOR::BLACK)];
    if (whiteRooks > 1)
        value += whiteRooks * factors[ROOK_PAIR_PENALTY].startValue;
    if (blackRooks > 1)
        value -= blackRooks * factors[ROOK_PAIR_PENALTY].startValue;
    return value - startValue;
}

int Analyser::evaluateQueens(int& value, const int& gameStage) const
{
    short startValue = value;
    value += (material->countQueens(COLOR::WHITE) - material->countQueens(COLOR::BLACK)) * factors[QUEEN_BASE_VALUE].startValue;
    return value - startValue;
}

int Analyser::evaluateSquareControl(int& value, const int& gameStage) const
{
    short startValue = value;

    value += control->ownCampControlDifference() * ownCampControlValue[gameStage];
    value += control->opponentCampControlDifference() * oppositeCampControlValue[gameStage];
    value += control->centerControlDifference() * centerControlValue[gameStage];

    return value - startValue;
}

int Analyser::evaluateMobility(int& value, const int& gameStage) const
{
    short startValue = value;
    if (material->countKnights(COLOR::WHITE) > 0)
        value += knightMobility[mobility->getMobility(COLOR::WHITE, PieceType::KNIGHT) / material->countKnights(COLOR::WHITE)][gameStage];
    if (material->countKnights(COLOR::BLACK) > 0)
        value -= knightMobility[mobility->getMobility(COLOR::BLACK, PieceType::KNIGHT) / material->countKnights(COLOR::BLACK)][gameStage];
    if (material->countBishops(COLOR::WHITE) > 0)
        value += bishopMobility[mobility->getMobility(COLOR::WHITE, PieceType::BISHOP) / material->countBishops(COLOR::WHITE)][gameStage];
    if (material->countBishops(COLOR::BLACK) > 0)
        value -= bishopMobility[mobility->getMobility(COLOR::BLACK, PieceType::BISHOP) / material->countBishops(COLOR::BLACK)][gameStage];
    if (material->countRooks(COLOR::WHITE) > 0)
        value += rookMobility[mobility->getMobility(COLOR::WHITE, PieceType::ROOK) / material->countRooks(COLOR::WHITE)][gameStage];
    if (material->countRooks(COLOR::BLACK) > 0)
        value -= rookMobility[mobility->getMobility(COLOR::BLACK, PieceType::ROOK) / material->countRooks(COLOR::BLACK)][gameStage];
    if (material->countQueens(COLOR::WHITE) > 0)
        value += queenMobility[mobility->getMobility(COLOR::WHITE, PieceType::QUEEN) / material->countQueens(COLOR::WHITE)][gameStage];
    if (material->countQueens(COLOR::BLACK) > 0)
        value -= queenMobility[mobility->getMobility(COLOR::BLACK, PieceType::QUEEN) / material->countQueens(COLOR::BLACK)][gameStage];
    return value - startValue;
}

int Analyser::evaluatePosition() const
{
    int value = 0;
    int gameStage = material->stageValueNormalised();
    evaluatePawns(value, gameStage);
    evaluateKnights(value, gameStage);
    evaluateBishops(value, gameStage);
    evaluateRooks(value, gameStage);
    evaluateQueens(value, gameStage);
    evaluateSquareControl(value, gameStage);
    evaluateMobility(value, gameStage);
    return value;
}

void Analyser::evaluateAll()
{
    for (PositionElement* element : elements)
        element->evaluate();
}




void Analyser::show() const
{
    int value = 0;
    int stage = material->stageValue();
    control->show();
}

