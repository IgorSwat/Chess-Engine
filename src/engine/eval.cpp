#include "eval.h"
#include "evalconfig.h"


namespace Evaluation {

    // --------------------------
    // Evaluation - main function
    // --------------------------

    Eval evaluate(const Board& board, NNUE& nnue)
    {
        // First, extract main evaluation score from NNUE
        Eval eval = Eval(nnue.forward(board));

        // Mating conditions
        // - To improve engine's abilities in finding mates, we apply a simple heuristic for certain mate endgames
        Color better_side = eval >= 0 ? board.side_to_move() : ~board.side_to_move();
        Color worse_side = ~better_side;

        // Mate position is simple to detect - worse side must be left with lonely king, and better side must have enough mating material
        if (Bitboards::singly_populated(board.pieces(worse_side)) &&
            (board.pieces(better_side, ROOK, QUEEN) || 
             Bitboards::popcount(board.pieces(better_side, KNIGHT, BISHOP)) >= 2 && board.pieces(better_side, BISHOP) ||
             Bitboards::popcount(board.pieces(better_side, KNIGHT)) >= 3))
        {
            // Interpolate a difference between mating eval and current eval
            Eval mate_eval = better_side == WHITE ? MAX_EVAL : -MAX_EVAL;
            Eval diff = mate_eval - eval;

            // First ingrediant - worse side's king distance to a chessboard corner
            // - Should be enough for now
            int corner_distance = 0;
            corner_distance += file_of(board.king_position(worse_side)) < FILE_E ? 
                                    file_of(board.king_position(worse_side)) - FILE_A : FILE_H - file_of(board.king_position(worse_side));
            corner_distance += rank_of(board.king_position(worse_side)) < RANK_5 ? 
                                    rank_of(board.king_position(worse_side)) - RANK_1 : RANK_8 - rank_of(board.king_position(worse_side));
            eval += diff / ((corner_distance + 1) * 2);
        }

        // Flatten eval according to distance to 50 move rule
        // - Punishes pointless shuffling and repeating the position
        eval = eval * (100 - board.halfmoves_c()) / 100;

        return eval;
    }

}