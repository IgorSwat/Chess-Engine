#pragma once

#include "eval.h"


/*
    ---------- Evaluation config ----------

    Contains evaluation-related constants
*/

namespace Evaluation {

    // -------------------------------------
    // Configuration - game stage parameters
    // -------------------------------------

    namespace GameStage {

        // Game stage is an integer from range [0, 256]
        using Stage = uint32_t;

        constexpr Stage MIN_GAME_STAGE = 0;     // Represents latest endgames - king and pawn endgames
        constexpr Stage MAX_GAME_STAGE = 256;   // Represents opening stage with all pieces still on the board

        // Parameters - piece stage influence
        constexpr Stage PieceWeights[PIECE_TYPE_RANGE] = {
            0,      
            0,      // Pawns
            9, 9,   // Knights & bishops
            14,     // Rook
            64,     // Queen
            0,      // King
            0
        };

        // Advanced stage definitions
        // - Just a helper constant based on the above paramaters
        constexpr Stage PAWN_ENDGAME = 0;
        constexpr Stage SINGLE_MINOR_VS_MINOR_ENDGAME = 2 * PieceWeights[KNIGHT];
        constexpr Stage SINGLE_ROOK_VS_ROOK_ENDGAME = 2 * PieceWeights[ROOK];
        constexpr Stage SINGLE_QUEEN_VS_QUEEN_ENDGAME = 2 * PieceWeights[QUEEN];
        constexpr Stage DOUBLE_MINOR_VS_MINOR_ENDGAME = 2 * PieceWeights[KNIGHT] + 2 * PieceWeights[BISHOP];
        constexpr Stage DOUBLE_ROOK_VS_ROOK_ENDGAME = 4 * PieceWeights[ROOK];
        constexpr Stage ROOK_VS_MINOR_ENDGAME = PieceWeights[KNIGHT] + PieceWeights[ROOK];
        constexpr Stage QUEEN_VS_ROOK_ENDGAME = PieceWeights[ROOK] + PieceWeights[QUEEN];

    }


    // ----------------------------------
    // Configuration - basic piece values
    // ----------------------------------

    // Since NNUE already covers piece imbalance factors, we use those values mostly for SEE and search purposes
    constexpr Eval PieceValues[PIECE_TYPE_RANGE] = {
        0,
        100,        // Pawn value
        350,        // Knight value - just a little bit more than 3 pawns
        350,        // Bishop value - just a little bit more than 3 pawns
        550,        // Rook value   - knight / bishop + 2 pawns
        1100,       // Queen value  - 2 rooks
    };

}