#pragma once

#include "evaluation.h"
#include "moveGeneration.h"
#include "see.h"
#include <algorithm>
#include <functional>


namespace MoveSelection {

    // -----------------
    // Global parameters
    // -----------------

    constexpr int MAX_NO_BUCKETS = 6;
    constexpr int MAX_NO_EXCLUDED_MOVES = 4;


    // --------------
    // Selector class
    // --------------

    // A generator-like class, which serves as intermediate layer between move generators and search crawlers
    class Selector
    {
    public:
        Selector(const BoardConfig* board, const Evaluation::Evaluator* evaluator = nullptr,
                 MoveGeneration::Phase gen = MoveGeneration::PSEUDO_LEGAL,
                 bool cascade = true)
            : board(board), evaluator(evaluator),
              cascade(cascade), gen(gen), strategy(this) { generateMoves(); }

        // Generator-style opertions
        EnhancedMove next(bool useStrategy = true);
        bool hasNext(bool test = true);

        // Restoring last generated move
        void restoreLastMove() { if (currentBucket != MAX_NO_BUCKETS) buckets[currentBucket] |= 0x1ULL << lastMoveID; }

        // Excluding moves
        void exclude(const Move& move) { excluded.push_back(move); }
        void includeAll() { excluded.clear(); }

        // Sorting
        // - Indexer creates an integer (key) for given move
        void sort(const std::function<int32_t(const Move&)>& indexer, EnhancementMode mode = EnhancementMode::CUSTOM_SORTING);

        // Getters and setters for private fields
        MoveGeneration::Phase phase() const { return gen; }

        // Customizable selector behavior - phase change
        // ---------------------------------------------

        bool cascade;

        // Customizable selector behavior - selection strategy
        // ---------------------------------------------------

        class Strategy
        {
        public:
            Strategy(const Selector* selector) : selector(selector) {}

            // Type definitions
            using Predicate = bool (Selector::*)(EnhancedMove&) const;
            using DecisionList = LightList<Predicate, MAX_NO_BUCKETS - 1>;

            // Dynamic rule change
            void addRule(MoveGeneration::Phase phase, Predicate pred) { decisionLists[phase].push_back(pred); }
            void clearRules(MoveGeneration::Phase phase) { decisionLists[phase].clear(); }
            void clearRules() { for (DecisionList& list : decisionLists) list.clear(); }

            // Assigns move to appropriate bucket based on defined rules
            std::uint8_t evaluate(MoveGeneration::Phase phase, EnhancedMove& move) const;

        private:
            // Selector connection
            const Selector* selector;

            // Representation of linearized decision tree for each phase
            DecisionList decisionLists[MoveGeneration::PHASE_RANGE];
        } strategy;

        // Selection predicates (selection strategy)
        // -----------------------------------------

        // Here is the list of all available selection predicates that can be applied in strategy class
        // Complex conditions (in the form of condition_condition) require both conditions to be fullfilled
        bool positiveSee(EnhancedMove& move) const { return SEE::evaluate_save(board, move) > 0; }
        bool neutralSee(EnhancedMove& move) const { return SEE::evaluate_save(board, move) == 0; }
        bool threatCreation(EnhancedMove& move) const { return evaluator->e_isCreatingThreats_c(move); }
        bool threatEvasion(EnhancedMove& move) const { return evaluator->e_isAvoidingThreats_c(move); }
        bool safe(EnhancedMove& move) const { return evaluator->e_isSafe_h(move); }
        bool neutralSee_threatCreation(EnhancedMove& move) const { return neutralSee(move) && threatCreation(move); }
        bool neutralSee_threatEvasion(EnhancedMove& move) const { return neutralSee(move) && threatEvasion(move); }
        bool safe_threatCreation(EnhancedMove& move) const { return safe(move) && threatCreation(move); }
        bool safe_threatEvasion(EnhancedMove& move) const { return safe(move) && threatEvasion(move); }
    
    private:
        // Helper functions
        void generateMoves();   // Generates moves according to current generation phase (gen)
        void resetSection();    // Reset selector to the begin (first batch of moves)
        void nextSection();     // Switch to the next batch of moves
        bool isExcluded(const Move& move) const { return std::find(excluded.begin(), excluded.end(), move) != excluded.end(); }

        // Board connection
        const BoardConfig* board;

        // Evaluator connection
        const Evaluation::Evaluator* evaluator;

        // Current generation phase logic
        MoveGeneration::Phase gen;

        // Move handling logic
        MoveList moves;
        EnhancedMove* sectionBegin;
        EnhancedMove* sectionEnd;
        EnhancedMove* nextMove;

        // Excluding moves from selectiom
        // Excluded moves are treated in the same way as illegal moves
        using Blacklist = LightList<Move, MAX_NO_EXCLUDED_MOVES>;

        Blacklist excluded;

        // Bucket is represented as a bitmask which determines the indices of moves belonging to given bucket
        // Each bucket contains moves of given quality, starting from most appealing moves (first bucket) to least appealing (last bucket)
        // Since buckets are only 64-bit in size, moves are divided into batches (1 batch = at most 64 moves)
        using Bucket = std::uint64_t;

        Bucket buckets[MAX_NO_BUCKETS] = { 0 };
        std::uint8_t currentBucket = 0;
        std::uint8_t currentBatch = 0;
        
        // A relative index to current batch
        std::uint8_t lastMoveID = 0;
    };


    // ------------------------------------
    // Predefined selection strategy makers
    // ------------------------------------

    void simple_ordering(Selector& selector);
    void standard_ordering(Selector& selector);
    void improved_ordering(Selector& selector);

}