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

        GAME_STAGE_MAX_VALUE = 256,
    };

    constexpr std::uint16_t PieceStageInfluence[PIECE_RANGE] = {
        0, 0, 8, 8, 16, 64, 0, 0,
        0, 0, 8, 8, 16, 64, 0, 0
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
    constexpr IValue PAWN_BASE_VALUE = { 100, 100 };
    constexpr IValue KNIGHT_BASE_VALUE = { 350, 350 };
    constexpr IValue BISHOP_BASE_VALUE = { 350, 350 };
    constexpr IValue ROOK_BASE_VALUE = { 450, 560 };
    constexpr IValue QUEEN_BASE_VALUE = { 1000, 1000 };

    // Knight-specyfic evaluation
    constexpr int MAX_CENTRAL_DENSITY = 16;

    constexpr Value KNIGHT_POSITION_DENSITY[MAX_CENTRAL_DENSITY + 1] = {
        -30, -25, -20, -15, -10, -5, 0, 5,
        10, 15, 20, 25, 30, 35, 40, 45,
        50
    };
    constexpr IValue KNIGHT_PAWN_SPREAD[8] = { {0, 30}, {0, 28}, {0, 25}, {0, 22}, {0, 17}, {0, 12}, {0, 6}, {0, 0} };

    // Bishop-specyfic evaluation
    constexpr IValue BISHOP_PAIR_VALUE = { 50, 50 };
    constexpr IValue BISHOP_ENEMY_PAWN_WEAKNESS = { 8, 46 };
    constexpr IValue BISHOP_OWN_PAWN_BLOCKAGE[7] = { {20, 10}, {18, 9}, {13, 7}, {5, 3}, {-7, -3}, {-23, -11}, {-40, -20} };
    constexpr IValue BISHOP_ENEMY_PAWN_BLOCKAGE[7] = { {42, -20}, {18, -11}, {6, -8}, {-6, -6}, {-14, -4}, {-22, -2}, {-28, 0} };

    // Rook-specyfic evaluation
    constexpr IValue ROOK_ON_SEMIOPEN_FILE = { 15, 3 };
    constexpr IValue ROOK_ON_OPEN_FILE = { 20, 5 };
    constexpr IValue ROOK_ON_78_RANK = { 25, 35 };
    constexpr IValue ROOK_ENEMY_PAWN_WEAKNESS = { 10, 64 };

    // Queen-specyfic evaluation
    constexpr IValue QUEEN_ENEMY_PAWN_WEAKNESS = { 15, 175 };

    // Piece mobility
    constexpr IValue KNIGHT_MOBILITY[9] = {
        {-27, -58}, {-12, -42}, {6, -21}, {12, -11}, {19, -2}, {27, 8}, {33, 15}, {39, 23},
        {45, 28}
    };
    constexpr IValue BISHOP_MOBILITY[14] = {
        {-23, -47}, {-14, -30}, {-7, -23}, {-1, -14}, {8, -1}, {18, 10}, {27, 22}, {32, 30},
        {36, 36}, {40, 43}, {43, 49}, {46, 54}, {49, 59}, {51, 64}
    };
    constexpr IValue ROOK_MOBILITY[15] = {
        {-10, -55}, {-4, -39}, {1, -27}, {4, -16}, {9, -7}, {16, 10}, {24, 30}, {29, 44},
        {33, 54}, {36, 62}, {39, 70}, {42, 78}, {45, 83}, {48, 90}, {50, 94}
    };
    constexpr IValue QUEEN_MOBILITY[28] = {
        {-6, -31}, {-4, -24}, {-2, -17}, {-1, -11}, {0, -3}, {1, 6}, {3, 24}, {5, 40},
        {7, 50}, {9, 69}, {13, 86}, {17, 98}, {24, 124}, {30, 136}, {35, 145}, {40, 150},
        {44, 155}, {47, 159}, {49, 161}, {50, 163}, {51, 166}, {51, 168}, {52, 170}, {52, 173},
        {53, 175}, {53, 176}, {53, 177}, {54, 179}
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
    constexpr IValue KNIGHT_OUTPOST_III_DEG = { 67, 18 };
    constexpr IValue BISHOP_OUTPOST_I_DEG = { 10, 2 };
    constexpr IValue BISHOP_OUTPOST_II_DEG = { 24, 6 };
    constexpr IValue BISHOP_OUTPOST_III_DEG = { 42, 10 };

    // Pawn structure points
    constexpr int GOOD_PAWN = -4;
    constexpr int UNDEFENDED_MOBILE_PAWN = 1;
    constexpr int DEFENDED_BLOCKED_PAWN = 2;
    constexpr int ISOLATED_MOBILE_PAWN = 7;
    constexpr int UNDEFENDED_BLOCKED_PAWN = 8;
    constexpr int ISOLATED_BLOCKED_PAWN = 15;
    constexpr int BACKWARD_PAWN = 15;

    constexpr IValue DOUBLED_PAWN_PENALTY = { -5, -30 };
    constexpr Value DOUBLED_PAWN_MAX_PENALTY = -70;
    constexpr IValue WEAK_PAWN_ATTACKED = { -8, -16 };

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
    constexpr int BLOCKED_PASSER_PENALTY_SIZE = -2; // Less = more severe penalty (based on PASSED_PAWN_POINTS table)
    constexpr int HEAVY_PIECE_BEHIND_PASSER[7] = { 50, 38, 24, 18, 10, 4, 4 };
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
    constexpr int KingTropismKnightPoints = -28;
    constexpr int KingTropismBishopPoints[24] = {
        -24, -24, -19, -13, -9, -7, -6, -5,
        -4, -3, -3, -3, -3, -3, -3, -3,
        -3, -3, -3, -3, -3, -3, -3, -3
    };
    constexpr int KingTropismRookPoints[24] = {
        -36, -36, -31, -23, -17, -11, -8, -6,
        -5, -5, -4, -4, -4, -4, -4, -4,
        -4, -4, -4, -4, -4, -4, -4, -4
    };
    constexpr int KingTropismQueenPoints[24] = {
        -76, -76, -58, -40, -26, -19, -16, -14,
        -12, -11, -10, -9, -9, -9, -9, -9,
        -9, -9, -9, -9, -9, -9, -9, -9
    };
    constexpr int KingAreaAttackWages[PIECE_TYPE_RANGE] = {
        0,
        0,  // Pawn
        -6,  // Knight
        -6,  // Bishop
        -8,  // Rook
        -12,  // Queen
        0,  // King
        0
    };

    constexpr IValue KING_SAFETY_VALUE = { 64, 20 };    // How much do 64 safety points worth (easier scaling)

    // Space
    constexpr int CENTRAL_SPACE_POINTS = 64;
    constexpr int OTHER_SPACE_POINTS = 38;

    constexpr IValue SPACE_TYPE_I = { 5, 1 };       // Per 64 space points (easier scaling)
    constexpr IValue SPACE_TYPE_II = { 9, 2 };      // Per 64 space points (easier scaling)


}