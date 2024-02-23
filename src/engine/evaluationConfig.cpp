#include "evaluationConfig.h"

namespace Evaluation {

    EvalParameter EvaluationConfig[EVAL_PARAMETERS_RANGE] = {
        {100, 100},         // PAWP_BASE_VALUE
        {350, 350},         // KNIGHT_BASE_VALUE
        {350, 350},         // BISHOP_BASE_VALUE
        {450, 560},         // ROOK_BASE_VALUE
        {1000, 1000},       // QUEEN_BASE_VALUE,

        {-30, -30},         // KNIGHT_DENSE_POSITION_MIN
        {50, 50},           // KNIGHT_DENSE_POSITION_MAX
        {0, 30},            // KNIGHT_PAWN_SPREAD_COMP
        {-27, -58},         // KNIGHT_MOBILITY_ZERO
        {43, 28},           // KNIGHT_MOBILITY_FULL
        {23, 5},            // KNIGHT_OUTPOST_I_DEG
        {40, 9},            // KNIGHT_OUTPOST_II_DEG
        {67, 18},           // KNIGHT_OUTPOST_III_DEG
        
        {50, 50},           // BISHOP_PAIR_BONUS
        {20, 10},           // BISHOP_OWN_PAWN_NO_BLOCKAGE
        {-40, -20},         // BISHOP_OWN_PAWN_FULL_BLOCKAGE
        {42, -20},          // BISHOP_ENEMY_PAWN_NO_BLOCKAGE
        {-28, 0},           // BISHOP_ENEMY_PAWN_FULL_BLOCKAGE
        {8, 46},            // BISHOP_ENEMY_PAWN_WEAKNESS
        {-23, -47},         // BISHOP_MOBILITY_ZERO
        {51, 64},           // BISHOP_MOBILITY_FULL
        {10, 2},            // BISHOP_OUTPOST_I_DEG
        {24, 6},            // BISHOP_OUTPOST_II_DEG
        {42, 10},           // BISHOP_OUTPOST_III_DEG

        {15, 30},           // ROOK_ON_SEMIOPEN_FILE_BONUS
        {20, 40},           // ROOK_ON_OPEN_FILE_BONUS
        {25, 50},           // ROOK_ON_78_RANK_BONUS
        {10, 64},           // ROOK_ENEMY_PAWN_WEAKNESS
        {-10, -55},         // ROOK_MOBILITY_ZERO
        {47, 90},           // ROOK_MOBILITY_FULL

        {15, 160},          // QUEEN_ENEMY_PAWN_WEAKNESS
        {-3, -31},         // QUEEN_MOBILITY_ZERO
        {49, 152},          // QUEEN_MOBILITY_FULL

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
        {0, -50},           // KING_PAWN_PROXIMITY_VALUE

        {5, 0},             // SPACE_BONUS
        {7, 0},             // UNCONTESTED_SPACE_BONUS
    };
    
}