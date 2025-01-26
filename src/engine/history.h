#pragma once

#include "searchConfig.h"
#include "../logic/boardConfig.h"


namespace Search {

    // -------------
    // History class
    // -------------

    // History heuristic is a dynamic move ordering algorithm, that focuses on amount of cut-offs cused by quiet moves in search tree
    // History container works similarly to transposition scores and should be shared among engine's crawlers
    class History
    {
    public:
        History() = default;

        using Score = int32_t;

        // Global modifiers
        void reset();
        void flatten(int factor = 1);   // History aging

        // Local modifiers
        // - <name>_t stands for trial table update, and <name>_h for history table update
        void update_t(Piece piece, Square to) { trials[piece][to]++; }
        void update_t(const BoardConfig* board, const Move& move) { update_t(board->onSquare(move.from()), move.to()); }
        void update_h(Piece piece, Square to, Score bonus) {
            Score clamped = std::clamp(bonus, -MAX_HISTORY, MAX_HISTORY);
            scores[piece][to] += clamped - scores[piece][to] * std::abs(clamped) / MAX_HISTORY;   // History gravity formula
        }
        void update_h(const BoardConfig* board, const Move& move, Score bonus) { update_h(board->onSquare(move.from()), move.to(), bonus); }

        // Getters
        // - count() returns only raw trial count for given move
        // - history() returns only raw history score for given move
        // - score() returns combined score from relative history formula (history & trials)
        Score count(Piece piece, Square to) const { return trials[piece][to]; }
        Score count(const BoardConfig* board, const Move& move) const { return count(board->onSquare(move.from()), move.to()); }
        Score history(Piece piece, Square to) const { return scores[piece][to]; }
        Score history(const BoardConfig* board, const Move& move) const { return history(board->onSquare(move.from()), move.to()); }
        Score score(Piece piece, Square to) const { 
            Score cnt = count(piece, to) + 1;   // Add 1 to prevent division by 0
            return scores[piece][to] * std::log2(cnt) / cnt;
        }
        Score score(const BoardConfig* board, const Move& move) const { return score(board->onSquare(move.from()), move.to()); }

    private:
        Score trials[PIECE_RANGE][SQUARE_RANGE] = { 0 };      // A count of tries of given move inside search tree
        Score scores[PIECE_RANGE][SQUARE_RANGE] = { 0 };      // A history scores for given move indexed with butterfly
    };

}