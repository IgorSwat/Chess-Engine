#pragma once

#include "movegen.h"
#include "../utilities/sarray.h"
#include <algorithm>
#include <functional>


/*
    ---------- Moveord ----------

    Tools for ordering and selection of moves
    - Implements both discreet (buckets) and continuous (standard sort) ordering of moves
*/

namespace MoveOrdering {

    // ----------------------------------------------
    // Move ordering - discrete - selector parameters
    // ----------------------------------------------

    // Maximum number of buckets in Selector class
    constexpr uint8_t MAX_BUCKETS = 4;

    // Maximum number of excluded moves in Selector class
    constexpr uint8_t MAX_EXCLUDED_MOVES = 4;


    // -----------------------------------
    // Move ordering - discrete - selector
    // -----------------------------------

    // Ordering & selection of moves
    // - Permament connection with given board
    // - Generator-style interface
    // - Implements discret ordering by dividing moves into buckets based on given conditions forming a mini decision tree
    class Selector
    {
    public:
        Selector(const Board* board, MoveGeneration::Mode gen = MoveGeneration::PSEUDO_LEGAL)
            : m_board(board), m_gen(gen) { generate_moves(); }

        // Selector behavior - generation phase switch strategy
        // - STRICT: Select moves only from current generation phase
        // - PARTIAL_CASCADE: Similar to STRICT, but changes the generation phase after first null move return (it works once per phase)
        // - FULL_CASCADE: Always changes generation phase and continues selection if there are no more moves from current phase
        enum Mode : uint32_t { STRICT = 0, PARTIAL_CASCADE, FULL_CASCADE };

        // Generator operations
        EMove next(Mode mode = FULL_CASCADE, bool use_strategy = true);
        bool has_next();

        // Restoring last generated move
        // - Uses bucket ordering idea to save element back to the currently explored bucket and quickly recover it
        void restore_last() { if (m_bucket != MAX_BUCKETS) m_buckets[m_bucket] |= 0x1ULL << m_last_move_id; }

        // Excluding moves
        void exclude(const Move& move) { m_excluded.push_back(move); }
        void include_all() { m_excluded.clear(); }
        bool is_excluded(const Move& move) const { return std::find(m_excluded.begin(), m_excluded.end(), move) != m_excluded.end(); }

        // Selection strategy
        // - Selection strategy is the main part of discrete move ordering
        // - Works as a compact decision tree which decides which bucket is appropriate for given move
        // - Can be created in dynamic way by adding or removing comparators
        // - Publicly available outside Selector class
        class Strategy
        {
        public:
            Strategy() = default;

            // Type definitions
            using Predicate = std::function<bool(EMove&)>;

            // Dynamic rules
            void add_rule(MoveGeneration::Mode mode, Predicate pred) { decisionLists[mode].push_back(pred); }
            void clear_rules(MoveGeneration::Mode mode) { decisionLists[mode].clear(); }
            void clear_rules() { for (auto& list : decisionLists) list.clear(); }

            // Assigns move to appropriate bucket based on defined rules
            uint32_t classify(MoveGeneration::Mode mode, EMove& move) const;

        private:
            // Representation of linearized decision tree for each phase
            StableArray<Predicate, MAX_BUCKETS - 1> decisionLists[MoveGeneration::MODE_RANGE];

        } strategy;

        // Friends
        friend void sort(Selector& selector, const std::function<int32_t(const Move& move)>& indexer, Moves::Enhancement enhancement_type);

    private:
        // Helper functions - move pool handlers
        void generate_moves();              // Generates moves according to m_gen mode
        void set_batch(uint8_t batch_id);
        void reset_batch() { set_batch(0); }

        // Board connection
        const Board* m_board;

        // Current generation phase logic
        MoveGeneration::Mode m_gen;

        // Move handling logic
        Moves::List<EMove> m_moves;
        EMove* m_section_begin;
        EMove* m_section_end;
        EMove* m_next_move;

        // Exclusion list
        StableArray<Move, MAX_EXCLUDED_MOVES> m_excluded;

        // Bucket is represented as a bitmask which determines the indices of moves belonging to given bucket
        // - Each bucket contains moves of given quality, starting from most appealing moves (first bucket) to least appealing (last bucket)
        // - Since buckets are only 64-bit in size, moves are divided into batches (portions of moves, 1 batch = at most 64 moves)
        using Bucket = std::uint64_t;

        Bucket m_buckets[MAX_BUCKETS] = { 0 };
        uint32_t m_bucket = 0;      // Current bucket
        uint32_t m_batch = 0;       // Current batch
        
        // A relative index to current batch
        uint32_t m_last_move_id = 0;
    };


    // ---------------------------------
    // Move ordering - continuous - sort
    // ---------------------------------

    // NOTE: all of the below sortings are in descending order

    // Indexer definition
    // - Indexer is a function that transforms a move into some integer, which then allows to compare different moves during sort
    // - NOTE: polymorphism allows this one to work for both Move and EMove objects
    using Indexer = std::function<int32_t(const Move& move)>;

    // Standard sort - for plain move list
    // - Not in place
    // - Less efficient, not recommended to use inside search routine
    template <unsigned size = 256>
    void sort(Moves::List<Move, size>& moves, const Indexer& indexer)
    {
        // An external table is necessary in this case
        std::pair<Move, int32_t> rtable[size];

        std::transform(moves.begin(), moves.end(), rtable, 
                       [](const Move& move) -> std::pair<Move, int32_t> { return std::make_pair(move, indexer(move)); });
        std::sort(rtable, rtable + moves.size(), 
                  [](const auto& a, const auto& b) -> bool { return a.second > b.second; });
        std::transform(rtable, rtable + moves.size(), moves.begin(),
                       [](const auto& a) -> Move { return a.first; });
    }

    // Standard sort - for enhanced move list
    // - In place, since we can store indices directly inside move objects
    // - Allows to choose the enhancement and therefore reuse the calculated keys in the future (see EMove specialized getters)
    template <unsigned size = 256>
    void sort(Moves::List<EMove, size>& moves, const Indexer& indexer, 
              Moves::Enhancement enhancement_type = Moves::Enhancement::CUSTOM_SORTING)
    {
        // First calculate indices and then sort
        std::for_each(moves.begin(), moves.end(), 
                      [&indexer, enhancement_type](EMove& move) -> void { move.enhance(enhancement_type, indexer(move)); });
        std::sort(moves.begin(), moves.end(),
                  [](EMove& a, EMove& b) -> bool { return a.key() > b.key(); });
    }

    // Standard sort - for move selector
    // - Since move selector is essentially an enhanced move list with dynamic range, we can treat it almost the same as above
    inline void sort(Selector& selector, const Indexer& indexer,
                     Moves::Enhancement enhancement_type = Moves::Enhancement::CUSTOM_SORTING)
    {
        // Similarly to above function, but with respect to selector's move range
        std::for_each(selector.m_next_move, selector.m_moves.end(), 
                      [&indexer, enhancement_type](EMove& move) -> void { move.enhance(enhancement_type, indexer(move)); });
        std::sort(selector.m_next_move, selector.m_moves.end(),
                  [](EMove& a, EMove& b) -> bool { return a.key() > b.key(); });
    }

}