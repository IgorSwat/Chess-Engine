#pragma once

#include "search.h"
#include <cinttypes>
#include <cstdlib>


// -------------------------
// Transposition table class
// -------------------------

class TranspositionTable
{
public:

    struct Entry
    {
        Entry() : key(0ULL), pieces(0xffffffffffffffff), depth(0), score(0), bestMove(), typeOfNode(Search::INVALID_NODE), age(0) {}
        Entry(std::uint64_t key, Bitboard pieces, Search::Depth depth, Value score, const Move& bestMove, Search::Node type, Search::Age age)
            : key(key), pieces(pieces), depth(depth), score(score), bestMove(bestMove), typeOfNode(type), age(age) {}

        bool matches(std::uint64_t key, Bitboard pieces) const
        {
            return this->key == key && this->pieces == pieces;
        }

        std::uint64_t key;              // Zobrist hash
        Bitboard pieces;                // For better collisions detection
        Search::Depth depth;            // Search depth used to obtain the score
        Value score;                    // Evaluation of the position
        Move bestMove;                  // Best move for side being on move
        Search::Node typeOfNode;        // Alpha-beta specyfic
        Search::Age age;                // Counted in halfmoves from starting position
    };

    void set(const Entry& entry, Search::Age rootAge);
    const Entry* probe(std::uint64_t key, Bitboard pieces) const;

private:
    std::uint64_t index(std::uint64_t key) const;

    // Adjustable hyperparameters
    static constexpr int MB_SIZE = 64;             // Total size of the transposition table in MB
    static constexpr int MASK_LENGTH = 21;         // For Entry of size 32 B

    // Data table
    Entry entries[1024 * 1024 * MB_SIZE / sizeof(Entry)];
    std::uint64_t mask = 0xffffffffffffffff >> (64 - MASK_LENGTH);
};


// ---------------------------
// Transposition table methods
// ---------------------------

inline void TranspositionTable::set(const Entry& entry, Search::Age rootAge)
{
    Entry* oldEntry = &entries[index(entry.key)];
    if (oldEntry->matches(entry.key, entry.pieces)) {
        if (oldEntry->depth < entry.depth)      // Deeper search is usualy more reliable
            (*oldEntry) = entry;
    }
    else if (Bitboards::popcount(oldEntry->pieces) > Bitboards::popcount(entry.pieces) || 
             Search::distance(rootAge, oldEntry->age) >= Search::distance(rootAge, entry.age))
        (*oldEntry) = entry;
}

inline const TranspositionTable::Entry* TranspositionTable::probe(std::uint64_t key, Bitboard pieces) const
{
    const Entry* entry = &entries[index(key)];
    return entry->matches(key, pieces) ? entry : nullptr;
}

inline std::uint64_t TranspositionTable::index(std::uint64_t key) const
{
    return key & mask;
}