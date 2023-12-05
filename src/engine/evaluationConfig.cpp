#include "evaluationConfig.h"

namespace Evaluation {

    Parameter EVALUATION_CONFIG[EVAL_PARAMETERS_RANGE] = {
        {100, 133},         // PAWP_BASE_VALUE
        {350, 441},         // KNIGHT_BASE_VALUE
        {375, 479},         // BISHOP_BASE_VALUE
        {500, 725},         // ROOK_BASE_VALUE
        {1150, 1500},       // QUEEN_BASE_VALUE,

        {30, 30},           // KNIGHT_PAWNS_BONUS
        {-5, -35},          // KNIGHT_DISTANT_PAWNS_PENALTY
        {22, 10},           // KNIGHT_OUTPOST_1_DEG_BONUS
        {49, 22},           // KNIGHT_OUTPOST_2_DEG_BONUS
        {65, 25},           // KNIGHT_OUTPOST_3_DEG_BONUS
        {-45, -61},         // KNIGHT_MOBILITY_ZERO
        {36, 21},           // KNIGHT_MOBILITY_FULL
        
        {50, 50},           // BISHOP_PAIR_BONUS
        {-50, -50},         // BAD_BISHOP_PENALTY
        {16, 5},            // BISHOP_FIANCHETTO_BONUS
        {50, 0},            // BISHOP_COLOR_WEAKNESS_BONUS
        {18, 8},            // BISHOP_OUTPOST_1_DEG_BONUS
        {38, 17},           // BISHOP_OUTPOST_2_DEG_BONUS
        {50, 20},           // BISHOP_OUTPOST_3_DEG_BONUS
        {-22, -51},         // BISHOP_MOBILITY_ZERO
        {50, 69},           // BISHOP_MOBILITY_FULL

        {15, 15},           // ROOK_ON_SEMIOPEN_FILE_BONUS
        {28, 28},           // ROOK_ON_OPEN_FILE_BONUS
        {48, 48},           // ROOK_ON_78_RANK_BONUS
        {-13, -43},         // ROOK_UNDEVELOPED_PENALTY
        {-25, -55},         // ROOK_MOBILITY_ZERO
        {40, 114},          // ROOK_MOBILITY_FULL

        {-20, -34},         // QUEEN_MOBILITY_ZERO
        {52, 151},          // QUEEN_MOBILITY_FULL

        {-8, -22},          // ISOLATED_PAWN_BASE_PENALTY
        {-7, -16},          // ISOLATED_PAWN_ATTACKED_PENALTY
        {-4, -19},          // DOUBLED_PAWN_PENALTY
        {-10, -30},         // BACKWARD_PAWN_BASE_PENALTY
        {-9, -20},          // BACKWARD_PAWN_ATTACKED_PENALTY
        {-3, -15},          // HANGING_PAWN_PENALTY
        {152, 196},         // PASSED_PAWN_MAX_BONUS
        {12, 32},           // CONNECTED_PASSER_BONUS

        {16, 0},            // PAWN_SHIELD_STRONG_BONUS
        {11, 0},            // PAWN_SHIELD_WEAKER_BONUS
        {-56, -11},         // PAWN_STORM_PENALTY
        {-20, 0},           // SEMIOPEN_FILE_NEAR_KING_PENALTY
        {-34, 0},           // OPEN_FILE_NEAR_KING_PENALTY
        {-260, 0},          // KING_CENTER_DISTANCE_MIN_VALUE
        {-600, -600},       // KING_AREA_ATTACKS_MAX_VALUE
        {80, 80},           // KING_AREA_ATTACKS_MAX_POINTS
        {0, -40},           // KING_PAWN_PROXIMITY_VALUE

        {5, 0},             // SPACE_BONUS
        {7, 0},             // UNCONTESTED_SPACE_BONUS
    };
    
}