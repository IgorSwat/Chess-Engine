#pragma once

#include "board.h"
#include "searchconfig.h"

/*
    ---------- History ----------

    An implementation of history heuristic mechanism
    - History as a shared object that aggregates all the move counters and history scores
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
        // - We use 64 bit integer to make sure that range of possible scores is big enough and there are no overflows
        // - Negative history is allowed
        using Score = int64_t;
        using Counter = uint32_t;

        // Global modifiers
        // - History malusements are implemented as flatten() method, which reduces all scores and counters by half (possibly multiple times)
        // - This method is called at the beginning of every new search and focuses on providing a balance between new and old history scores
        // - Real reduction is 2^factor
        void flatten(unsigned factor = 1);
        void reset() { flatten(64); }

        // Local modifiers
        // - Dynamic update of history tables
        void update_score(Piece piece, Square to, Score bonus) { 
            Score clamped = std::clamp(bonus, -MAX_HISTORY_SCORE, MAX_HISTORY_SCORE);
            m_scores[piece][to] += clamped - m_scores[piece][to] * std::abs(clamped) / MAX_HISTORY_SCORE;   // History gravity formula

            // Launch auto-flattening when some value reaches it's maximum
            if (m_scores[piece][to] == MAX_HISTORY_SCORE)
                flatten(1);
        }
        void update_score(const Board& board, const Move& move, Score bonus) { update_score(board.on(move.from()), move.to(), bonus); }
        void update_counter(Piece piece, Square to) { m_counters[piece][to]++; }
        void update_counter(const Board& board, const Move& move) { update_counter(board.on(move.from()), move.to()); }

        // Getters
        // - count() returns only raw count for given move
        // - score() returns only raw history score for given move
        // - history() returns combined score from relative history formula (history & trials)
        Score score(Piece piece, Square to) const { return m_scores[piece][to]; }
        Score score(const Board& board, const Move& move) { return score(board.on(move.from()), move.to()); }
        Counter count(Piece piece, Square to) const { return m_counters[piece][to]; }
        Counter count(const Board& board, const Move& move) { return count(board.on(move.from()), move.to()); }
        Score history(Piece piece, Square to) const { return score(piece, to) * score(piece, to) / (count(piece, to) + 1); }
        Score history(const Board& board, const Move& move) const { return history(board.on(move.from()), move.to()); }

    private:
        // History tables - move scores & counters
        // - Improved indexing - instead of butterfly index, we use piece-square approach
        alignas(32) Score m_scores[PIECE_RANGE][SQUARE_RANGE] = { 0 };        // Sum of all history bonuses for given move
        alignas(16) Counter m_counters[PIECE_RANGE][SQUARE_RANGE] = { 0 };    // How many times move was tried in search tree
    };

}