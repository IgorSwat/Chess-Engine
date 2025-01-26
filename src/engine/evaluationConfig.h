#pragma once

#include "../logic/types.h"
#include <cinttypes>
#include <functional>
#include <cmath>


using Value = std::int32_t;

namespace Evaluation {

    // ----------
    // Game stage 
    // ----------

    enum GameStage : std::uint16_t {
        OPENING = 200, MIDGAME = 128, ENDGAME = 0,

        PAWN_ENDGAME = 0,
        SINGLE_MINOR_ENDGAME = 9,
        SINGLE_ROOK_ENDGAME = 14,
        SINGLE_MINOR_VS_MINOR_ENDGAME = 18,
        SINGLE_ROOK_VS_ROOK_ENDGAME = 28,
        DOUBLE_MINOR_VS_MINOR_ENDGAME = 36,
        DOUBLE_ROOK_VS_ROOK_ENDGAME = 56,
        ROOK_VS_MINOR_ENDGAME = 23,
        QUEEN_VS_ROOK_ENDGAME = 78,

        GAME_STAGE_MAX_VALUE = 256,
    };

    constexpr std::uint16_t PieceStageInfluence[PIECE_RANGE] = {
        0, 0, 9, 9, 14, 64, 0, 0,
        0, 0, 9, 9, 14, 64, 0, 0
    };


    // ------------------------
    // Other evaluation defines
    // ------------------------

    struct IValue   // Game stage interpolated parameter
    {
        constexpr IValue() : opening(0), endgame(0), diff(0) {}
        constexpr IValue(Value opening, Value endgame) : opening(opening), endgame(endgame), diff(endgame - opening) {}

        Value opening;
        Value endgame;
        Value diff;
    };


    // ---------------------
    // Evaluation parameters
    // ---------------------

    // Piece base values
    constexpr IValue PAWN_BASE_VALUE = { 100, 125 };
    constexpr IValue KNIGHT_BASE_VALUE = { 350, 475 };
    constexpr IValue BISHOP_BASE_VALUE = { 375, 505 };
    constexpr IValue ROOK_BASE_VALUE = { 400, 750 };
    constexpr IValue QUEEN_BASE_VALUE = { 900, 1500 };

    // Knight-specyfic evaluation
    constexpr int MAX_CENTRAL_DENSITY = 16;

    constexpr Value KNIGHT_POSITION_DENSITY[MAX_CENTRAL_DENSITY + 1] = {
        -20, -17, -14, -10, -6, -5, 0, 3,
        5, 8, 11, 15, 20, 25, 30, 35,
        40
    };
    constexpr IValue KNIGHT_PAWN_SPREAD[8] = { {0, 30}, {0, 28}, {0, 25}, {0, 22}, {0, 17}, {0, 12}, {0, 6}, {0, 0} };

    // Bishop-specyfic evaluation
    constexpr IValue BISHOP_PAIR_VALUE = { 60, 75 };
    constexpr IValue BISHOP_ENEMY_PAWN_WEAKNESS = { 5, 24 };
    constexpr IValue BISHOP_OWN_PAWN_BLOCKAGE[7] = { {20, 10}, {18, 9}, {13, 7}, {5, 3}, {-8, -4}, {-26, -13}, {-48, -23} };
    constexpr IValue BISHOP_ENEMY_PAWN_BLOCKAGE[7] = { {32, -20}, {14, -11}, {5, -8}, {-6, -6}, {-14, -4}, {-22, -2}, {-28, 0} };

    // Rook-specyfic evaluation
    constexpr IValue ROOK_ON_SEMIOPEN_FILE = { 15, 3 };
    constexpr IValue ROOK_ON_OPEN_FILE = { 20, 5 };
    constexpr IValue ROOK_ON_78_RANK = { 20, 30 };
    constexpr IValue ROOK_ENEMY_PAWN_WEAKNESS = { 0, 20 };

    // Queen-specyfic evaluation
    constexpr IValue QUEEN_ENEMY_PAWN_WEAKNESS = { 0, 70 };

    // Piece mobility
    constexpr IValue KNIGHT_MOBILITY[9] = {
        {-31, -58}, {-12, -42}, {6, -21}, {10, -11}, {17, -2}, {23, 8}, {30, 15}, {36, 23},
        {40, 28}
    };
    constexpr IValue BISHOP_MOBILITY[14] = {
        {-26, -47}, {-9, -34}, {-4, -13}, {-1, -3}, {8, 0}, {18, 8}, {27, 22}, {32, 30},
        {36, 36}, {40, 43}, {43, 49}, {46, 54}, {49, 59}, {51, 64}
    };
    constexpr IValue ROOK_MOBILITY[15] = {
        {-20, -75}, {-6, -27}, {-2, -12}, {-2, -4}, {-1, 0}, {0, 7}, {2, 20}, {8, 29},
        {13, 40}, {20, 52}, {27, 54}, {40, 56}, {46, 63}, {51, 70}, {56, 78}
    };
    constexpr IValue QUEEN_MOBILITY[28] = {
        {-6, -31}, {-4, -19}, {-2, -10}, {-1, -3}, {0, -1}, {1, 5}, {3, 15}, {5, 29},
        {7, 47}, {9, 66}, {11, 85}, {14, 98}, {17, 124}, {19, 136}, {22, 145}, {24, 150},
        {26, 155}, {28, 159}, {29, 161}, {30, 163}, {30, 166}, {31, 168}, {31, 170}, {31, 173},
        {31, 175}, {31, 176}, {31, 177}, {32, 179}
    };

    // Outposts
    constexpr int OutpostFactors[COLOR_RANGE][SQUARE_RANGE] = {
        // For white side
        {0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
         1, 2, 2, 3, 3, 2, 2, 1,
         2, 4, 7, 10, 10, 7, 4, 2,
         3, 8, 13, 14, 14, 13, 8, 3,
         4, 10, 15, 16, 16, 15, 10, 4,
         3, 9, 11, 13, 13, 11, 9, 3,
         2, 4, 5, 6, 6, 5, 4, 2},

         // For black side
        {2, 4, 5, 6, 6, 5, 4, 2,
         3, 9, 11, 13, 13, 11, 9, 3,
         4, 10, 15, 16, 16, 15, 10, 4,
         3, 8, 13, 14, 14, 13, 8, 3,
         2, 4, 7, 10, 10, 7, 4, 2,
         1, 2, 2, 3, 3, 2, 2, 1,
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0}
    };

    constexpr IValue KNIGHT_OUTPOST_I_DEG = { 23, 5 };
    constexpr IValue KNIGHT_OUTPOST_II_DEG = { 40, 9 };
    constexpr IValue KNIGHT_OUTPOST_III_DEG = { 65, 15 };
    constexpr IValue BISHOP_OUTPOST_I_DEG = { 10, 2 };
    constexpr IValue BISHOP_OUTPOST_II_DEG = { 24, 6 };
    constexpr IValue BISHOP_OUTPOST_III_DEG = { 42, 10 };

    // Pawn structure points
    constexpr int GOOD_PAWN = -6;
    constexpr int UNDEFENDED_MOBILE_PAWN = 2;
    constexpr int DEFENDED_BLOCKED_PAWN = 3;
    constexpr int ISOLATED_MOBILE_PAWN = 6;
    constexpr int UNDEFENDED_BLOCKED_PAWN = 8;
    constexpr int ISOLATED_BLOCKED_PAWN = 12;
    constexpr int BACKWARD_PAWN = 12;

    constexpr IValue DOUBLED_PAWN_PENALTY = { -1, -40 };
    constexpr Value DOUBLED_PAWN_MAX_PENALTY = -80;

    // Passed pawns
    constexpr int PASSED_PAWN_POINTS[7][32] = {
        // Estra distance level for boundary safety
        {152, 152, 152, 152, 152, 152, 152, 152, 152, 152, 155, 162, 170, 198, 227, 272, 360 /*Base*/, 396, 412, 420, 425, 425, 425, 425, 425, 425, 425, 425, 425, 425, 425, 425},

        // One square away from promotion
        {72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 74, 78, 86, 100, 132, 176 /*Base*/, 192, 204, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208},

        {40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 41, 43, 48, 58, 74, 98 /*Base*/, 108, 113, 116, 118, 118, 118, 118, 118, 118, 118, 118, 118, 118, 118, 118},

        {28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 29, 31, 34, 39, 48, 64 /*Base*/, 70, 73, 75, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76, 76},

        {20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21, 24, 30, 40 /*Base*/, 45, 48, 50, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52},

        {20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21, 24 /*Base*/, 27, 29, 30, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31},

        // On a starting square
        {20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20 /*Base*/, 23, 25, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26}
    };
    constexpr int BLOCKED_PASSER_PENALTY_SIZE = -3; // Less = more severe penalty (based on PASSED_PAWN_POINTS table)
    constexpr int HEAVY_PIECE_BEHIND_PASSER[7] = { 36, 28, 18, 12, 7, 3, 2 };
    constexpr int CONNECTED_PASSERS_FACTOR = 20;    // To balance division by 1024 (2^10)

    constexpr IValue PASSED_PAWNS_VALUE = { 64, 64 };    // How much do 64 passer points worth (easier scaling)

    // Proximity factors
    constexpr int OUR_PAWN_PROXIMITY_FACTOR = 2;
    constexpr int ENEMY_PAWN_PROXIMITY_FACTOR = 3;
    constexpr int OUR_PASSER_PROXIMITY_FACTOR = 8;
    constexpr int ENEMY_PASSER_PROXIMITY_FACTOR = 12;

    constexpr IValue KING_PAWN_PROXIMITY_VALUE = { 0, -50 };

    // King safety
    constexpr int PawnShieldPoints[64] = {
        0, 10, 12, 38, 10, 32, 38, 64,
        2, 6, 32, 40, 24, 26, 60, 68,
        6, 30, 12, 44, 30, 50, 44, 72,
        12, 32, 34, 46, 38, 54, 50, 76,
        2, 24, 32, 60, 6, 26, 40, 68,
        6, 8, 34, 62, 8, 12, 62, 72,
        12, 38, 34, 50, 32, 54, 46, 76,
        16, 42, 36, 64, 42, 60, 64, 80
    };
    constexpr int PawnStormPoints[7] = {
        0, -30, -28, -21, -12, -3, -1
    };
    constexpr int KingTropismBishopPoints[24] = {
        -24, -24, -19, -13, -9, -7, -6, -5,
        -4, -3, -3, -3, -3, -3, -3, -3,
        -3, -3, -3, -3, -3, -3, -3, -3
    };
    constexpr int KingTropismRookPoints[24] = {
        -24, -24, -20, -14, -10, -8, -6, -5
        -4, -4, -3, -3, -3, -3, -3, -3,
        -3, -3, -3, -3, -3, -3, -3, -3
    };
    constexpr int KingTropismQueenPoints[24] = {
        -94, -93, -81, -66, -49, -43, -37, -31,
        -25, -22, -19, -17, -15, -15, -15, -15,
        -15, -15, -15, -15, -15, -15, -15, -15
    };
    constexpr int KING_TROPISM_THRESHOLD = -90;
    constexpr int KingAreaAttackWages[PIECE_TYPE_RANGE] = {
        0,
        0,  // Pawn
        -8,  // Knight
        -7,  // Bishop
        -10,  // Rook
        -14,  // Queen
        0,  // King
        0
    };

    constexpr IValue KING_SAFETY_VALUE = { 60, 0 };    // How much do 64 safety points worth (easier scaling)

    // Space
    constexpr int CENTRAL_SPACE_POINTS = 64;
    constexpr int OTHER_SPACE_POINTS = 42;

    constexpr IValue SPACE_TYPE_I = { 2, 0 };           // Per 64 space points (easier scaling)
    constexpr IValue SPACE_TYPE_II = { 8, 0 };          // Per 64 space points (easier scaling)

    // Threats
    constexpr bool EVALUATE_HNGING_PAWN_THREATS = true;
    constexpr int THREAT_POINTS[PIECE_TYPE_RANGE] = { 0, 12, 20, 20, 24, 30, 0, 0 };

    constexpr IValue THREAT_VALUE = { -64, -64 };         // Per 64 space points (easier scaling)

    // Tempo bonus
    constexpr IValue TEMPO_BONUS = { 24, 8 };

    // Evaluation descent
    constexpr int PAWN_COMPLEXITY_POINTS = 42;
    constexpr int PASSED_PAWN_COMPLEXITY_POINTS = 55;
    constexpr int PIECE_COMPLEXITY_POINTS = 85;
    constexpr int INFILTRATION_COMPLEXITY_POINTS = 64;
    constexpr int PAWNS_ON_BOTH_SIDES_COMPLEXITY_POINTS = 58;

    constexpr int COMPLEXITY_THRESHOLD = 1200;
    constexpr int PAWN_ENDGAME_COMPLEXITY_THRESHOLD = 120;

    constexpr int PieceImbalancePoints[PIECE_TYPE_RANGE] = { 0, 0, 60, 60, 106, 240, 0, 0 };

    // Special case evaluation adjustment
    constexpr Value OPPOSITE_COLOR_BISHOPS_PAWN_ADJUSTMENT = 70;    // Negative adjustment
    constexpr Value DRAWISH_ROOK_ENDGAME_ADJUSTMENT = 55;
    constexpr Value UP_A_PIECE_ENDGAME_ADJUSTMENT = 0;
}