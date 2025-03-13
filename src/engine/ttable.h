#pragma once

#include "search.h"
#include "eval.h"


// -----------------------------------
// Helper functions - size calculation
// -----------------------------------

// Helper function - constexpr log2 with upper bound round
constexpr inline uint32_t ceil_log2(uint32_t n, uint32_t p = 0)
{
    return (1U << p) >= n ? p : ceil_log2(n, p + 1);
}


// --------------------------------
// Transposition table - parameters
// --------------------------------

// Transposition table size (MB)
// - Main parameter
// - This is an upper bound. Because TT size must be a power of 2, the real size satisfies condition: TT_SIZE_MB/2 < size <= TT_SIZE_MB
constexpr uint32_t TT_SIZE_MB = 128;


// -------------------
// Transposition table
// -------------------

class TranspositionTable
{
public:
    TranspositionTable() = default;

    // Entry definition
    struct Entry
    {
        // Key attributes
        Zobrist::Hash key = 0;                  // Main key part - enables to compare positions
        Bitboard pieces = 0xffffffffffffffff;   // Additional key part - for more reliable comparisions and better collisions detection

        bool matches(std::uint64_t key, Bitboard pieces) const { return this->key == key && this->pieces == pieces; }
        bool matches(const Entry& other) const { return matches(other.key, other.pieces); }

        // Replacement strategy components
        // - NOTE: both root_age and age are absolute values measured from the beginning of chess game
        Search::Age root_age = 0;       // Together with pieces allows to compare two entries for having the same root (= search tree)
        Search::Age age = 0;            // Age of position related to this entry

        bool same_search_tree(Search::Age root_age, Bitboard pieces) const { return this->root_age == root_age && this->pieces == pieces; }
        bool same_search_tree(const Entry& other) const { return same_search_tree(other.root_age, other.pieces); }

        // Search results
        Search::Depth depth = 0;                // Search depth used to obtain the score
        Search::Node node_type = Search::INVALID_NODE;
        Search::Score score = 0;
        Move best_move = Moves::null;

        // Other things
        Evaluation::Eval static_eval = Evaluation::NO_EVAL;
    };

    // Transposition table - update
    void set(const Entry& entry) {
        Entry* old_entry = &m_entries[index(entry.key)];

        if (old_entry->matches(entry)) {
            if (old_entry->depth < entry.depth)      // Deeper search is usualy more reliable
                (*old_entry) = entry;
        }
        else if (!old_entry->same_search_tree(entry) || entry.age < old_entry->age ||
                 entry.age == old_entry->age && entry.node_type == Search::PV_NODE)
            (*old_entry) = entry;
    }

    // Transposition table - probe
    const Entry* probe(std::uint64_t key, Bitboard pieces) const {
        const Entry* entry = &m_entries[index(key)];
        return entry->matches(key, pieces) ? entry : nullptr; 
    }

    // Transposition table - other parameters
    // - Completely dependable on TT_SIZE_MB
    // - NOTE: Transposition table size must be a power of 2 to enable creating a proper key from Zobrist hash
    static constexpr uint32_t SIZE = 1024 * 1024 * TT_SIZE_MB / (1U << ceil_log2(sizeof(Entry)));
    static constexpr uint32_t MASK_LENGTH = 20 + ceil_log2(TT_SIZE_MB / (1U << ceil_log2(sizeof(Entry))));
    static constexpr uint64_t MASK = 0xffffffffffffffff >> (64 - MASK_LENGTH);

private:
    // Helper function - calculating index from Zobrist hash
    static uint64_t index(uint64_t key) { return key & MASK; }

    // Main data table
    Entry m_entries[SIZE];
};