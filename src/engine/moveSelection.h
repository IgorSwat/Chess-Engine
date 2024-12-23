#pragma once

#include "evaluation.h"
#include "moveGeneration.h"
#include "see.h"
#include <functional>


namespace MoveSelection {

    // ------------------------
    // Selection behavior flags
    // ------------------------

    // Selection strategy is defined as 64 bit integer, with 8 consecutive bits representing strategy for given generation phase
    // For example, 8 least significant bits defines strategy for QUIET (moves) phase
    using Strategy = std::uint64_t;
    using Substrategy = Strategy;

    constexpr int MAX_NO_BUCKETS = 5;
    constexpr int SUBSTRATEGY_LENGTH = 8;   // bits
    constexpr Substrategy SUBSTRATEGY_MASK = 0xff;

    // Local strategies - listed by descending significance
    constexpr Substrategy POSITIVE_SEE = 0x1;
    constexpr Substrategy NEUTRAL_SEE_THREAT_CREATION = POSITIVE_SEE << 1;  // SEE = 0 + creates threat
    constexpr Substrategy NEUTRAL_SEE_THREAT_EVASION = POSITIVE_SEE << 2;   // SEE = 0 + evades threat
    constexpr Substrategy NEUTRAL_SEE = POSITIVE_SEE << 3;                  // Just SEE = 0
    constexpr Substrategy NEGATIVE_SEE_THREAT_CREATION = POSITIVE_SEE << 4;
    constexpr Substrategy NEGATIVE_SEE_THREAT_EVASION = POSITIVE_SEE << 5;

    // Creating strategies
    constexpr inline Strategy make_strategy(Substrategy substrategy, MoveGeneration::Phase gen)
    {
        return substrategy << (gen - 1) * SUBSTRATEGY_LENGTH;
    }

    constexpr inline Substrategy extract_substrategy(Strategy strategy, MoveGeneration::Phase gen)
    {
        return (strategy >> (gen - 1) * SUBSTRATEGY_LENGTH) & SUBSTRATEGY_MASK;
    }

    // Complex strategies
    constexpr Strategy SIMPLE_ORDERING = 0;
    constexpr Strategy STANDARD_ORDERING = 
        make_strategy(POSITIVE_SEE | NEUTRAL_SEE, MoveGeneration::CAPTURE) |
        make_strategy(POSITIVE_SEE | NEUTRAL_SEE, MoveGeneration::CHECK_EVASION);
    constexpr Strategy IMPROVED_ORDERING = 
        make_strategy(POSITIVE_SEE | NEUTRAL_SEE_THREAT_EVASION | NEUTRAL_SEE | NEGATIVE_SEE_THREAT_CREATION, MoveGeneration::CAPTURE) |
        make_strategy(NEUTRAL_SEE_THREAT_CREATION | NEUTRAL_SEE_THREAT_EVASION | NEUTRAL_SEE | NEGATIVE_SEE_THREAT_CREATION, MoveGeneration::QUIET_CHECK) |
        make_strategy(NEUTRAL_SEE_THREAT_CREATION | NEUTRAL_SEE_THREAT_EVASION | NEUTRAL_SEE | NEGATIVE_SEE_THREAT_CREATION, MoveGeneration::QUIET) |
        make_strategy(POSITIVE_SEE | NEUTRAL_SEE_THREAT_EVASION | NEUTRAL_SEE, MoveGeneration::CHECK_EVASION);


    // --------------
    // Selector class
    // --------------

    // A generator-like class, which serves as intermediate layer between move generators and search crawlers
    class Selector
    {
    public:
        Selector(const BoardConfig* board, const Evaluation::Evaluator* evaluator = nullptr,
                 MoveGeneration::Phase gen = MoveGeneration::PSEUDO_LEGAL,
                 MoveSelection::Strategy strategy = MoveSelection::SIMPLE_ORDERING,
                 bool cascade = true)
            : board(board), evaluator(evaluator),
              strategy(strategy), cascade(cascade), gen(gen) { generateMoves(); }

        // Generator-style opertions
        EnhancedMove next();
        bool hasNext();

        // Sorting
        // - Indexer creates an integer (key) for given move
        void sort(const std::function<int32_t(const Move&)>& indexer, EnhancementMode mode = EnhancementMode::CUSTOM_SORTING);

        // Getters for private fields
        MoveGeneration::Phase phase() const { return gen; }

        // Customizable selector behavior
        MoveSelection::Strategy strategy;
        bool cascade;
    
    private:
        // Helper functions
        void generateMoves();   // Generates moves according to current generation phase (gen)
        void resetSection();    // Reset selector to the begin (first batch of moves)
        void nextSection();     // Switch to the next batch of moves

        // Board connection
        const BoardConfig* board;

        // Evaluator connection
        const Evaluation::Evaluator* evaluator;

        // Current generation phase
        MoveGeneration::Phase gen;

        // Move handling logic
        MoveList moves;
        EnhancedMove* sectionBegin;
        EnhancedMove* sectionEnd;
        EnhancedMove* nextMove;

        // Bucket is represented as a bitmask which determines the indices of moves belonging to given bucket
        // Each bucket contains moves of given quality, starting from most appealing moves (first bucket) to least appealing (last bucket)
        // Since buckets are only 64-bit in size, moves are divided into batches (1 batch = at most 64 moves)
        using Bucket = std::uint64_t;

        Bucket buckets[MAX_NO_BUCKETS] = { 0 };
        std::uint8_t currentBucket = 0;
        std::uint8_t currentBatch = 0;
    };

}