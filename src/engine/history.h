#pragma once

#include "searchConfig.h"
#include "../logic/boardConfig.h"
#include <algorithm>


namespace Search {

    // -------------
    // History class
    // -------------

    // History heuristic is a dynamic move ordering algorithm, that focuses on amount of cut-offs cused by quiet moves in search tree
    // History container works similarly to transposition table and should be shared among engine's crawlers
    class History
    {
    public:
        History() = default;

        using Score = std::int32_t;

        void reset() { for (int i = 0; i < PIECE_RANGE; i++) std::fill(table[i], table[i] + SQUARE_RANGE, 0); }
        void flatten() { for (int i = 0; i < PIECE_RANGE; i++) for (int j = 0; j < SQUARE_RANGE; j++) table[i][j] /= 2; }

        void update(Piece piece, Square to, Score bonus) {
            Score clamped = std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY);
            table[piece][to] += clamped - table[piece][to] * std::abs(clamped) / MAX_HISTORY;   // History gravity formula
        }
        void update(const BoardConfig* board, const Move& move, Score bonus) { update(board->onSquare(move.from()), move.to(), bonus); }

        Score score(Piece piece, Square to) const { return table[piece][to]; }
        Score score(const BoardConfig* board, const Move& move) const { return score(board->onSquare(move.from()), move.to()); }

    private:
        Score table[PIECE_RANGE][SQUARE_RANGE] = { 0 };
    };

}