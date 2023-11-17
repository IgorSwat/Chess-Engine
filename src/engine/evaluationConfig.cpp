#include "evaluationConfig.h"

namespace Evaluation {

    Parameter EVALUATION_CONFIG[EVAL_PARAMETERS_RANGE] = {
        {100, 133},         // PAWP_BASE_VALUE
        {350, 437},         // KNIGHT_BASE_VALUE
        {375, 468},         // BISHOP_BASE_VALUE
        {500, 700},         // ROOK_BASE_VALUE
        {1150, 1450},       // QUEEN_BASE_VALUE,

        {30, 30},           // KNIGHT_PAWNS_BONUS
        {-5, -35},          // KNIGHT_DISTANT_PAWNS_PENALTY
        {22, 10},           // KNIGHT_OUTPOST_1_DEG_BONUS
        {52, 23},           // KNIGHT_OUTPOST_2_DEG_BONUS
        {65, 25},           // KNIGHT_OUTPOST_3_DEG_BONUS
        {-45, -61},         // KNIGHT_MOBILITY_ZERO
        {36, 21},           // KNIGHT_MOBILITY_FULL
        
        {50, 50},           // BISHOP_PAIR_BONUS
        {-50, -50},           // BAD_BISHOP_PENALTY
        {16, 5},            // BISHOP_FIANCHETTO_BONUS
        {50, 0},            // BISHOP_COLOR_WEAKNESS_BONUS
        {18, 8},            // BISHOP_OUTPOST_1_DEG_BONUS
        {42, 18},           // BISHOP_OUTPOST_2_DEG_BONUS
        {50, 20},           // BISHOP_OUTPOST_3_DEG_BONUS
        {-22, -47},         // BISHOP_MOBILITY_ZERO
        {50, 69},           // BISHOP_MOBILITY_FULL

        {15, 15},           // ROOK_ON_SEMIOPEN_FILE_BONUS
        {28, 28},           // ROOK_ON_OPEN_FILE_BONUS
        {48, 48},           // ROOK_ON_78_RANK_BONUS
        {-20, -44},         // ROOK_UNDEVELOPED_PENALTY
        {-25, -50},         // ROOK_MOBILITY_ZERO
        {40, 114},          // ROOK_MOBILITY_FULL

        {-20, -34},         // QUEEN_MOBILITY_ZERO
        {52, 151},          // QUEEN_MOBILITY_FULL
    };
    
}