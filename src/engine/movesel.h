#pragma once

#include "movegen.h"
#include "../utilities/sarray.h"
#include <functional>


/*
    ---------- Movesel ----------

    Tools for ordering and selection of moves
    - Most functionalities focused in Selector class
    - Implements both discreet (buckets) and continuous (standard sort) ordering of moves
*/

namespace MoveSelection {

    // -------------------
    // Selector parameters
    // -------------------

    // Maximum number of buckets in Selector class
    constexpr uint8_t MAX_BUCKETS = 8;

    // Maximum number of excluded moves in Selector class
    constexpr uint8_t MAX_EXCLUDED_MOVES = 4;


    // --------------
    // Selector class
    // --------------

    // Ordering & selection of moves
    // - Permament connection with given board
    // - Generator-style interface
    // - Implements discret ordering by dividing moves into buckets based on given conditions forming a mini decision tree
    // - Implements continuous ordering by sorting moves by given dimension (which could be some move property, like SEE evaluation)
    // - Applies all key functionalities from MoveGeneration
    class Selector
    {
    public:
        Selector(const Board::Board* board, MoveGeneration::Mode gen = MoveGeneration::PSEUDO_LEGAL)
            : m_board(board), m_gen(gen) {}

        // Selector behavior - generation phase switch strategy
        // - STRICT: Select moves only from current generation phase
        // - PARTIAL_CASCADE: Similar to STRICT, but changes the generation phase after first null move return (it works once per phase)
        // - FULL_CASCADE: Always changes generation phase and continues selection if there are no more moves from current phase
        enum Mode : std::uint8_t { STRICT = 0, PARTIAL_CASCADE, FULL_CASCADE };

        // Generator-style opertions
        Move next(Mode mode = FULL_CASCADE, bool use_strategy = true);
        bool has_next(bool test = true);

        // Restoring last generated move
        // - Uses bucket ordering idea to save element back to the currently explored bucket and quickly recover it
        void restore_last() { if (m_bucket != MAX_BUCKETS) m_buckets[m_bucket] |= 0x1ULL << m_last_move_id; }

        // Excluding moves
        void exclude(const Move& move) { m_excluded.push_back(move); }
        void include_all() { m_excluded.clear(); }

        // Sorting moves
        // void sort(const std::function<int32_t(const Move&)>& indexer, EnhancementMode mode = EnhancementMode::CUSTOM_SORTING);

    private:
        // Board connection
        const Board::Board* m_board;

        // Current generation phase logic
        MoveGeneration::Mode m_gen;

        // Move handling logic
        Moves::List m_moves;
        Move* m_section_begin;
        Move* m_section_end;
        Move* m_next_move;

        // Exclusion list
        // - Max 4 excluded moves
        StableArray<Move, MAX_EXCLUDED_MOVES> m_excluded;

        // Bucket is represented as a bitmask which determines the indices of moves belonging to given bucket
        // Each bucket contains moves of given quality, starting from most appealing moves (first bucket) to least appealing (last bucket)
        // Since buckets are only 64-bit in size, moves are divided into batches (1 batch = at most 64 moves)
        using Bucket = std::uint64_t;

        Bucket m_buckets[MAX_BUCKETS] = { 0 };
        std::uint8_t m_bucket = 0;      // Current bucket
        std::uint8_t m_batch = 0;       // Current batch
        
        // A relative index to current batch
        std::uint8_t m_last_move_id = 0;
    };

}