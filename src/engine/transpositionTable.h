#pragma once

#include "evaluation.h"
#include <cinttypes>


using Depth = std::uint8_t;
using Age = std::uint16_t;

enum NodeType : std::uint8_t { PV_NODE = 0, CUT_NODE, ALL_NODE, INVALID_NODE };


// Adjustable hyperparameters
constexpr int mbSize = 32;
constexpr int maskLength = 20;

class TranspositionTable
{
public:
    struct Entry
    {
        Entry() : key(0ULL), pieces(0xffffffffffffffff), depth(0), score(0), bestMove(), typeOfNode(INVALID_NODE), age(0) {}
        Entry(std::uint64_t key, Bitboard pieces, Depth depth, Value score, const Move& bestMove, NodeType type, Age age)
            : key(key), pieces(pieces), depth(depth), score(score), bestMove(bestMove), typeOfNode(type), age(age) {}

        std::uint64_t key;      // Zobrist hash
        Bitboard pieces;        // For better collisions detection
        Depth depth;
        Value score;
        Move bestMove;
        NodeType typeOfNode;
        Age age;                // Counted in halfmoves from starting position
    };

    TranspositionTable() : mask(0xffffffffffffffff >> (64 - maskLength)) {}

    void set(Entry&& entry, Age rootAge);
    const Entry* probe(std::uint64_t key, Bitboard pieces) const;

private:
    std::uint64_t index(std::uint64_t key) const;
    Age ageMetric(Age age, Age rootAge) const;
    bool matches(const Entry& entry, std::uint64_t key, Bitboard pieces) const;

    Entry entries[32 * 1024 * mbSize];
    std::uint64_t mask;
};


inline void TranspositionTable::set(Entry&& entry, Age rootAge)
{
    Entry* oldEntry = &entries[index(entry.key)];
    if (matches(*oldEntry, entry.key, entry.pieces)) {
        if (oldEntry->depth < entry.depth)      // Deeper search is usualy more reliable
            (*oldEntry) = std::move(entry);
    }
    else if (Bitboards::popcount(oldEntry->pieces) > Bitboards::popcount(entry.pieces) || 
        ageMetric(oldEntry->age, rootAge) >= ageMetric(entry.age, rootAge))
        (*oldEntry) = std::move(entry);
}

inline const TranspositionTable::Entry* TranspositionTable::probe(std::uint64_t key, Bitboard pieces) const
{
    const Entry* entry = &entries[index(key)];
    return matches(*entry, key, pieces) ? entry : nullptr;
}

inline std::uint64_t TranspositionTable::index(std::uint64_t key) const
{
    return key & mask;
}

inline Age TranspositionTable::ageMetric(Age age, Age rootAge) const
{
    return age > rootAge ? age - rootAge : rootAge - age;
}

inline bool TranspositionTable::matches(const Entry& entry, std::uint64_t key, Bitboard pieces) const
{
    return entry.key == key && entry.pieces == pieces;
}