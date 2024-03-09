#pragma once

#include "interpolation.h"
#include "../logic/boardConfig.h"
#include <iostream>


namespace Evaluation {

    // ----------------
    // Helper functions
    // ---------------- 

    template <typename Functional>
    inline void fill_params_table(EvalParameter* table, int size, EvalParameter start, EvalParameter end, Functional func)
    {
        for (int i = 0; i <= size; i++)
            table[i] = Interpolation::interpolate(start, end, func, float(i) / size);
    }

    template <Color side, bool show>
    inline void add_eval(Value& value, Value incr, const char* featureName)
    {
        if constexpr (show)
            std::cout << "Feature \"" << featureName << "\" evaluated for side " << side << " as: " << incr << std::endl;
        value += incr;
    }


    // ---------------------
    // Main Evaluation class
    // ---------------- ----

    class Evaluator
    {
    public:
        Evaluator(BoardConfig* board, EvalParameter* parameters) : board(board), parameters(parameters) { initEvalTables(); }

        Value evaluate();

    private:
        // Evaluation components
        void initEvalTables();
        template <Color side> void initCommonData();
        template <Color side> Value evaluatePawns();
        template <Color side> Value evaluatePieces();
        template <Color side> Value evaluateKing();

        // Helper methods
        template <Color side> void updateProximity(Square sq, int ourFactor, int enemyFactor);
        template <Color side> void updateKingAreaSafety(Square sq, Bitboard attacks, int attackFactor);

        // Connected board
        BoardConfig* board;

        // Evaluation parameters
        EvalParameter* parameters;
        EvalParameter KnightDensePosition[17] = { };
        EvalParameter KnightPawnSpread[8] = { };
        EvalParameter KnightMobility[9] = { };
        EvalParameter BishopOwnPawnBlockage[7] = { };
        EvalParameter BishopEnemyPawnBlockage[7] = { };
        EvalParameter BishopMobility[14] = { };
        EvalParameter RookMobility[15] = { };
        EvalParameter QueenMobility[28] = { };
        
        // Common calculations shared among different evaluation methods
        std::uint16_t stage = GAME_STAGE_MAX_VALUE;

        // Piece-attack properties
        Bitboard attacks[COLOR_RANGE][PIECE_TYPE_RANGE] = { 0 };

        // Pawn structure properties
        int pawnRankSpread = 0;         // Horizontal spread
        int mostAdvancedUnstopablePasser = 8;     // Distance to promotion of the most advanced passed pawn
        int structurePoints[COLOR_RANGE][SQUARE_COLOR_RANGE] = { 0 };

        // King proximity
        int proximityPoints[COLOR_RANGE] = { 0 };
        int proximityWages[COLOR_RANGE] = { 0 };

        // King safety related
        int safetyPoints[COLOR_RANGE] = { 0 };
        int kingAreaAttackers[COLOR_RANGE] = { 0 };             // (Number of attackers) - (Number of defenders) difference
        int kingAreaAttackPoints[COLOR_RANGE] = { 0 };          // Absolute weighted count of each attack on king area
        Bitboard kingFrontSpans[COLOR_RANGE][2] = { 0 };        // First and second rank span areas in front of king
        Bitboard kingArea[COLOR_RANGE] = { 0 };                 // King area consists of king position square and surrounding squares attacked by king
        Bitboard weakKingSpan[COLOR_RANGE] = { 0 };             // An area consisting of king area and second front span, that is not defended twice by ally pawns

        // Other common properties
        int centralDensity = 0;
        int boardDensity = 0;

    };


    template <Color side>
    inline void Evaluator::updateProximity(Square sq, int ourFactor, int enemyFactor)
    {
        constexpr Color enemy = ~side;
        proximityPoints[side] += Board::SquareDistance[sq][board->kingPosition(side)] * ourFactor;
        proximityWages[side] += ourFactor;
        proximityPoints[enemy] += Board::SquareDistance[sq][board->kingPosition(enemy)] * enemyFactor;
        proximityWages[enemy] += enemyFactor;
    }

    template <Color side>
    inline void Evaluator::updateKingAreaSafety(Square sq, Bitboard attacks, int attackFactor)
    {
        constexpr Color enemy = ~side;
        Bitboard kingAreaAttacks = attacks & kingArea[enemy];
        if (kingAreaAttacks) {                                      // Enemy king (offensive piece)
            kingAreaAttackers[enemy]++;
            kingAreaAttackPoints[enemy] += attackFactor * Bitboards::popcount(kingAreaAttacks);
        }
        else if (attacks & weakKingSpan[enemy])
            kingAreaAttackers[enemy]++;
        if (kingFrontSpans[side][0] & (attacks | sq))               // Our king (defensive piece)
            kingAreaAttackers[side]--;
    }

}