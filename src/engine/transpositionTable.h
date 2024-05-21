#pragma once

#include "evaluation.h"
#include <cinttypes>
#include <cmath>


using Depth = std::uint8_t;
using Age = std::uint16_t;
enum NodeType : std::uint8_t { PV_NODE = 0, CUT_NODE, ALL_NODE, INVALID_NODE };

inline Age metric(Age age, Age rootAge)
{
    return age > rootAge ? age - rootAge : rootAge - age;
}


// Adjustable hyperparameters
constexpr int MB_SIZE = 32;             // Total size of the transposition table in MB
constexpr int MASK_LENGTH = 20;         // For Entry of size 32 B


class TranspositionTable
{
public:

    struct Entry
    {
        Entry() : key(0ULL), pieces(0xffffffffffffffff), depth(0), score(0), bestMove(), typeOfNode(INVALID_NODE), age(0) {}
        Entry(std::uint64_t key, Bitboard pieces, Depth depth, Value score, const Move& bestMove, NodeType type, Age age)
            : key(key), pieces(pieces), depth(depth), score(score), bestMove(bestMove), typeOfNode(type), age(age) {}

        bool matches(std::uint64_t key, Bitboard pieces) const
        {
            return this->key == key && this->pieces == pieces;
        }

        std::uint64_t key;      // Zobrist hash
        Bitboard pieces;        // For better collisions detection
        Depth depth;            // Search depth used to obtain the score
        Value score;            // Evaluation of the position
        Move bestMove;          // Best move for side being on move
        NodeType typeOfNode;    // Alpha-beta specyfic
        Age age;                // Counted in halfmoves from starting position
    };

    void set(const Entry& entry, Age rootAge);
    const Entry* probe(std::uint64_t key, Bitboard pieces) const;

private:
    std::uint64_t index(std::uint64_t key) const;

    Entry entries[1024 * 1024 * MB_SIZE / sizeof(Entry)];
    std::uint64_t mask = 0xffffffffffffffff >> (64 - MASK_LENGTH);
};


inline void TranspositionTable::set(const Entry& entry, Age rootAge)
{
    Entry* oldEntry = &entries[index(entry.key)];
    if (oldEntry->matches(entry.key, entry.pieces)) {
        if (oldEntry->depth < entry.depth)      // Deeper search is usualy more reliable
            (*oldEntry) = entry;
    }
    else if (Bitboards::popcount(oldEntry->pieces) > Bitboards::popcount(entry.pieces) || metric(oldEntry->age, rootAge) >= metric(entry.age, rootAge))
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