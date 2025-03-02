#pragma once

#include "board.h"


/*
    ---------- Evaluation ----------

    An implementation of so called HCE (Hand Craften Evaluation) for static evaluation of the position
    - Most of evaluation functionality concentrated in Evaluator class, dynamically linked to the board
    - Evaluation value is a single integer value representing position evaluation in centipawns (100 centipawns = 1 pawn)
    - Evaluation produces side effects, which allows to answer more advanced questions about the position
*/

namespace Evaluation {

    // -----------------------------
    // Evaluation - type definitions
    // -----------------------------

    // The range of evaluation values is limited and specified in corresponding config file
    using Eval = int32_t;


    // ----------------------------
    // Evaluation - Evaluator class
    // ----------------------------

    // TODO: add after developing the NNUE architecture

}