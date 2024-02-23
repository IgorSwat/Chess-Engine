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


    // ---------------------
    // Evaluation parameters
    // ---------------------

    struct EvalParameter
    {
        EvalParameter() : opening(0), diff(0) {}
        EvalParameter(Value opening, Value endgame) : opening(opening), diff(endgame - opening) {}

        Value opening;
        Value diff;
        Value endgame() const { return opening + diff; }
    };

    enum EvalParameters {
        PAWN_BASE_VALUE = 0,
        KNIGHT_BASE_VALUE,
        BISHOP_BASE_VALUE,
        ROOK_BASE_VALUE,
        QUEEN_BASE_VALUE,

        KNIGHT_DENSE_POSITION_MIN,
        KNIGHT_DENSE_POSITION_MAX,
        KNIGHT_PAWN_SPREAD_COMP,
        KNIGHT_MOBILITY_ZERO,
        KNIGHT_MOBILITY_FULL,
        KNIGHT_OUTPOST_I_DEG,
        KNIGHT_OUTPOST_II_DEG,
        KNIGHT_OUTPOST_III_DEG,

        BISHOP_PAIR_BONUS,
        BISHOP_OWN_PAWN_NO_BLOCKAGE,
        BISHOP_OWN_PAWN_FULL_BLOCKAGE,
        BISHOP_ENEMY_PAWN_NO_BLOCKAGE,
        BISHOP_ENEMY_PAWN_FULL_BLOCKAGE,
        BISHOP_ENEMY_PAWN_WEAKNESS,
        BISHOP_MOBILITY_ZERO,
        BISHOP_MOBILITY_FULL,
        BISHOP_OUTPOST_I_DEG,
        BISHOP_OUTPOST_II_DEG,
        BISHOP_OUTPOST_III_DEG,

        ROOK_ON_SEMIOPEN_FILE_BONUS,
        ROOK_ON_OPEN_FILE_BONUS,
        ROOK_ON_78_RANK_BONUS,
        ROOK_ENEMY_PAWN_WEAKNESS,
        ROOK_MOBILITY_ZERO,
        ROOK_MOBILITY_FULL,

        QUEEN_ENEMY_PAWN_WEAKNESS,
        QUEEN_MOBILITY_ZERO,
        QUEEN_MOBILITY_FULL,

        ISOLATED_PAWN_BASE_PENALTY,
        ISOLATED_PAWN_ATTACKED_PENALTY,
        DOUBLED_PAWN_PENALTY,
        BACKWARD_PAWN_BASE_PENALTY,
        BACKWARD_PAWN_ATTACKED_PENALTY,
        HANGING_PAWN_PENALTY,
        PASSED_PAWN_MAX_BONUS,
        CONNECTED_PASSER_BONUS,

        PAWN_SHIELD_STRONG_BONUS,
        PAWN_SHIELD_WEAKER_BONUS,
        PAWN_STORM_PENALTY,
        SEMIOPEN_FILE_NEAR_KING_PENALTY,
        OPEN_FILE_NEAR_KING_PENALTY,
        KING_CENTER_DISTANCE_MIN_VALUE,
        KING_AREA_ATTACKS_MAX_VALUE,
        KING_AREA_ATTACKS_MAX_POINTS,
        KING_PAWN_PROXIMITY_VALUE,

        SPACE_BONUS,
        UNCONTESTED_SPACE_BONUS,

        EVAL_PARAMETERS_RANGE = 54
    };

    extern EvalParameter EvaluationConfig[EVAL_PARAMETERS_RANGE];


    // ------------------------
    // Other evaluation defines
    // ------------------------

    enum OutpostType {
        I_DEG_OUTPOST = 0, II_DEG_OUTPOST, III_DEG_OUTPOST,

        OUTPOST_TYPE_RANGE = 3
    };

}