#pragma once

#include "board.h"
#include "nnue.h"


/*
    ---------- Evaluation ----------

    An implementation of so called HCE (Hand Craften Evaluation) for static evaluation of the position
    - Most of evaluation functionality concentrated in Evaluator class, dynamically linked to the board
    - Evaluation value is a single integer value representing position evaluation in centipawns (100 centipawns = 1 pawn)
    - Evaluation produces side effects, which allows to answer more advanced questions about the position
    - Used to contain more code before NNUE introduction
*/

namespace Evaluation {

    // -----------------------------
    // Evaluation - type definitions
    // -----------------------------

    // The range of evaluation values is limited and specified in corresponding config file
    using Eval = int32_t;


    // --------------------------
    // Evaluation - main function
    // --------------------------

    // This function utilizes NNUE to evaluate given position
    // However, value retrieved from NNUE is not the only evaluation factor
    // - Basically an adapter for NNUE, which takes into consideration other things like evaluation descent (approaching 50 move rule)
    Eval evaluate(const Board& board, NNUE& nnue);


    // -----------------------------
    // Evaluation - helper functions
    // -----------------------------

    inline Eval relative_eval(Eval eval, const Board& board)
    {
        return board.side_to_move() == WHITE ? eval : -eval; 
    }

}

using Evaluation::Eval;