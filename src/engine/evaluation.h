#pragma once

#include "interpolation.h"
#include "../logic/boardConfig.h"
#include <iostream>


namespace Evaluation {

    // ----------------
    // Helper functions
    // ---------------- 

    template <Color side, bool show>
    inline void add_eval(Value& value, Value incr, const char* featureName)
    {
        if constexpr (show)
            std::cout << "Feature \"" << featureName << "\" evaluated for side " << side << " as: " << incr << std::endl;
        value += incr;
    }


    // ---------------------
    // Main Evaluation class
    // ---------------------

    class Evaluator
    {
    public:
        Evaluator(BoardConfig* board) : board(board) {}

        // Main evaluation function
        Value evaluate();

        // Position info obtained after evaluation
        int threats(Color side) const;

    private:
        // Evaluation components
        template <Color side> void initCommonData();
        template <Color side> Value evaluatePawns1();   // First call, before evaluatePieces()
        template <Color side> Value evaluatePawns2();   // Second call, after evaluatePieces()
        template <Color side> Value evaluatePieces();
        template <Color side> Value evaluateKingAndMisc();
        Value adjustEval(Value eval) const;

        // Helper methods
        int countAttackers(Bitboard area, Color side, PieceType type) const;
        template <typename... PieceTypes> int countAttackers(Bitboard area, Color side, PieceType type, PieceTypes... types) const;

        template <Color side> void updateProximity(Square sq, int ourFactor, int enemyFactor);
        template <Color side> void updateKingAreaSafety(Square sq, Bitboard attacks, int attackFactor);

        // Connected board
        BoardConfig* board;

        // Common calculations shared among different evaluation methods
        std::uint16_t stage = GAME_STAGE_MAX_VALUE;

        // Piece imabalance
        int pieceCount[COLOR_RANGE][PIECE_TYPE_RANGE] = { 0 };  // Count all the pieces except for the kings

        // Piece-attack properties
        Bitboard attacks[COLOR_RANGE][PIECE_TYPE_RANGE] = { 0 };
        Bitboard multipleAttacks[COLOR_RANGE][PIECE_TYPE_RANGE] = { 0 };

        // Pawn structure properties
        int pawnRankSpread = 0;         // Horizontal spread
        int mostAdvancedUnstopablePasser = 8;     // Distance to promotion of the most advanced passed pawn
        int structurePoints[COLOR_RANGE][SQUARE_COLOR_RANGE] = { 0 };
        int passerPoints[SQUARE_RANGE] = { 0 };
        Bitboard weakPawns[COLOR_RANGE] = { 0 };
        Bitboard passedPawns[COLOR_RANGE] = { 0 };

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

        // Threats
        int threatCount[COLOR_RANGE] = { 0 };                   // Threats for side X means threats that side Y generates against X

        // Other common properties
        int centralDensity = 0;
        int boardDensity = 0;

    };


    inline int Evaluator::threats(Color side) const
    {
        return threatCount[side];
    }

    inline int Evaluator::countAttackers(Bitboard area, Color side, PieceType type) const
    {
        return multipleAttacks[side][type] & area ? 2 : bool(attacks[side][type] & area);
    }

    template <typename... PieceTypes>
    inline int Evaluator::countAttackers(Bitboard area, Color side, PieceType type, PieceTypes... types) const
    {
        return countAttackers(area, side, type) + countAttackers(area, side, types...);
    }


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