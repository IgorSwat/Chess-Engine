#pragma once

#include "board.h"
#include "searchconfig.h"

/*
    ---------- History ----------

    An implementation of history heuristic mechanism
    - Core of the idea comes from well known reinforcement learning k-bandit problem
    - It is named history for tradition, even though it is significantly different than classical history implementation
    - Scores are aggregated in static tables similarly to transposition table entries
    - For each move, the score is calculated using formula: Q(M, n + 1) = Q(M, n) + importance * (R(M) - Q(M, n))
*/

namespace Search {

    // -------
    // History
    // -------

    // This class works similarly to transposition table - it is menaged by the engine and shared among all the crawlers
    class History 
    {
    public:
        // History types definition
        // - We can look at score as a [0, 1] floating point value quantized into [0, S] integer range for efficiency purposes
        using Score = int16_t;

        // Global modifiers
        // - Allow to reset the whole history table and forget about anything it learned
        void reset() { for (int i = 0; i < PIECE_RANGE; i++) for (int j = 0; j < SQUARE_RANGE; j++) Q[i][j] = 0; }
        void flatten(int c = 1) { for (int i = 0; i < PIECE_RANGE; i++) for (int j = 0; j < SQUARE_RANGE; j++) Q[i][j] >>= c;}

        // Local modifiers
        // - Dynamic update of history table
        // - Utilizes Q(M, n + 1) = Q(M, n) + importance * (R(M) - Q(M, n)) formula
        // - importance = (c * depth) / (ply * start_depth) where c is a hyperparameter defined in searchconfig.h
        // - Additional division by 100 to normalize c factor (which is an integer instead of [0, 1] float)
        void update(Piece piece, Square to, int c, int8_t start_depth, int8_t depth, int16_t ply, Score R) { 
            Q[piece][to] = Q[piece][to] + int64_t(c) * depth * (R - Q[piece][to]) / (100 * (ply + 1) * start_depth);
        }
        void update(const Board& board, const Move& move, int c, int8_t start_depth, int8_t depth, int16_t ply, Score R) {
            update(board.on(move.from()), move.to(), c, start_depth, depth, ply, R);
        }

        // Getters
        Score score(Piece piece, Square to) const { return Q[piece][to]; }
        Score score (const Board& board, const Move& move) const { return score(board.on(move.from()), move.to()); }

    private:
        // History tables - move scores
        // - Improved indexing - instead of butterfly index, we use piece-square approach
        Score Q[PIECE_RANGE][SQUARE_RANGE] = { 0 };        // Move scores
    };

}