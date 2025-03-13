#include "eval.h"


namespace Evaluation {

    // --------------------------
    // Evaluation - main function
    // --------------------------

    Eval evaluate(const Board& board, NNUE& nnue)
    {
        // First, extract main evaluation score from NNUE
        Eval eval = Eval(nnue.forward(board));

        // Flatten eval according to distance to 50 move rule
        // - Punishes pointless shuffling and repeating the position
        eval = eval * (100 - board.halfmoves_c()) / 100;

        return eval;
    }

}