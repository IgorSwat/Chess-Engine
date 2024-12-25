#pragma once

#include "interpolation.h"
#include "../logic/boardConfig.h"
#include <iostream>


namespace Evaluation {

    // ---------------------
    // Main Evaluation class
    // ---------------------

    class Evaluator
    {
    public:
        Evaluator(const BoardConfig* board) : board(board) {}

        // Main evaluation functions
        Value evaluate();

        // Additional position info. The following syntax is being used:
        // - e_<method_name>: a pre-evaluation is required for method to work properly
        // - <method_name>_c: a correct method that always returns an exact result
        // - <method_name>_h: a heuristic method that most often (but not always) returns an exact result
        bool oppositeColorBishops_c() const;
        int e_countThreatsAgainst_c(Color side) const { return threatCount[side]; }
        Bitboard e_threatsAgainst_c(Color side) const { return threatMap[side]; }
        bool e_isCreatingThreats_c(const Move& move) const;
        bool e_isAvoidingThreats_c(const Move& move) const;
        bool e_isSafe_h(const Move& move) const;

    private:
        // Evaluation components
        template <Color side> void initCommonData();
        template <Color side> Value evaluatePawns1();   // First call, before evaluatePieces()
        template <Color side> Value evaluatePawns2();   // Second call, after evaluatePieces()
        template <Color side> Value evaluatePieces();
        template <Color side> Value evaluateKingAndMisc();

        // Evaluation descent
        Value adjustEval(Value eval) const;

        // Helper functions
        int countAttackers(Bitboard area, Color side, PieceType type) const;
        template <typename... PieceTypes> int countAttackers(Bitboard area, Color side, PieceType type, PieceTypes... types) const;

        template <Color side> void updateProximity(Square sq, int ourFactor, int enemyFactor);
        template <Color side> void updateKingAreaSafety(Square sq, Bitboard attacks, int attackFactor);

        // Board connection
        const BoardConfig* board;

        // Stage & material imbalance
        std::uint16_t stage = GAME_STAGE_MAX_VALUE;
        int pieceCount[COLOR_RANGE][PIECE_TYPE_RANGE] = { 0 };  // Count all the pieces except for the kings

        // Piece-attack properties
        Bitboard attacks[COLOR_RANGE][PIECE_TYPE_RANGE] = { 0 };
        Bitboard multipleAttacks[COLOR_RANGE][PIECE_TYPE_RANGE] = { 0 };

        // Threats
        int threatCount[COLOR_RANGE] = { 0 };
        Bitboard threatMap[COLOR_RANGE] = { 0 };                    // Bitboards of all threatened pieces from given side
        Bitboard safetyMap[COLOR_RANGE][PIECE_TYPE_RANGE] = { 0 };  // Bitboards of safe squares (not threatened) for each type of piece

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
        int safetyPoints[COLOR_RANGE] = { 0 };                  // Points against given side
        int tropismPoints[COLOR_RANGE] = { 0 };                 // Points against given side
        int kingAreaAttackers[COLOR_RANGE] = { 0 };             // (Number of attackers) - (Number of defenders) difference
        int kingAreaAttackPoints[COLOR_RANGE] = { 0 };          // Absolute weighted count of each attack on king area
        Bitboard kingFrontSpans[COLOR_RANGE][2] = { 0 };        // First and second rank span areas in front of king
        Bitboard kingArea[COLOR_RANGE] = { 0 };                 // King area consists of king position square and surrounding squares attacked by king
        Bitboard weakKingSpan[COLOR_RANGE] = { 0 };             // An area consisting of king area and second front span, that is not defended twice by ally pawns

        // Other properties
        int centralDensity = 0;
        int boardDensity = 0;

    };


    // -----------------
    // Evaluator methods
    // -----------------

    inline int Evaluator::countAttackers(Bitboard area, Color side, PieceType type) const
    {
        return multipleAttacks[side][type] & area ? 2 : bool(attacks[side][type] & area);
    }

    template <typename... PieceTypes>
    inline int Evaluator::countAttackers(Bitboard area, Color side, PieceType type, PieceTypes... types) const
    {
        return countAttackers(area, side, type) + countAttackers(area, side, types...);
    }


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

}