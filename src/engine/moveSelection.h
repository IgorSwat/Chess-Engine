#pragma once

#include "evaluation.h"
#include "moveGeneration.h"
#include "see.h"
#include <functional>


namespace MoveSelection {

    // -----------------
    // Global parameters
    // -----------------

    constexpr int MAX_NO_BUCKETS = 5;


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
        bool hasNext();

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
        bool threatCreation(EnhancedMove& move) const { return evaluator->isCreatingThreats(move); }
        bool threatEvasion(EnhancedMove& move) const { return evaluator->isAvoidingThreats(move); }
        bool neutralSee_threatCreation(EnhancedMove& move) const { return SEE::evaluate_save(board, move) == 0 && 
                                                                          evaluator->isCreatingThreats(move); }
        bool neutralSee_ThreatEvasion(EnhancedMove& move) const { return SEE::evaluate_save(board, move) == 0 && 
                                                                         evaluator->isAvoidingThreats(move); }

    
    private:
        // Helper functions
        void generateMoves();   // Generates moves according to current generation phase (gen)
        void resetSection();    // Reset selector to the begin (first batch of moves)
        void nextSection();     // Switch to the next batch of moves

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

        // Bucket is represented as a bitmask which determines the indices of moves belonging to given bucket
        // Each bucket contains moves of given quality, starting from most appealing moves (first bucket) to least appealing (last bucket)
        // Since buckets are only 64-bit in size, moves are divided into batches (1 batch = at most 64 moves)
        using Bucket = std::uint64_t;

        Bucket buckets[MAX_NO_BUCKETS] = { 0 };
        std::uint8_t currentBucket = 0;
        std::uint8_t currentBatch = 0;
    };


    // ------------------------------------
    // Predefined selection strategy makers
    // ------------------------------------

    void simple_ordering(Selector& selector);
    void standard_ordering(Selector& selector);
    void improved_ordering(Selector& selector);

}