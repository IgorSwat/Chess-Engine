#pragma once

#include "evaluation.h"
#include "moveGeneration.h"
#include "see.h"
#include <functional>


namespace MoveSelection {

    // ------------------------
    // Selection behavior flags
    // ------------------------

    // Selection strategy is defined as 64 bit integer, with 10 consecutive bits representing strategy for given generation phase
    // For example, first 10 bits defines strategy for QUIET (moves) phase
    using Strategy = std::uint64_t;
    using Substrategy = Strategy;

    // Local strategies
    constexpr Substrategy STRATEGY_BASE = 0x1;
    constexpr Substrategy SUBSTRATEGY_MASK = 0x3ff;

    constexpr Substrategy POSITIVE_SEE = STRATEGY_BASE;
    constexpr Substrategy ZERO_SEE = STRATEGY_BASE << 1;
    constexpr Substrategy THREAT_CREATION = STRATEGY_BASE << 2;
    constexpr Substrategy THREAT_EVASION = STRATEGY_BASE << 3;

    // Creating strategies
    constexpr inline Strategy make_strategy(Substrategy substrategy, MoveGeneration::MoveGenType gen)
    {
        return substrategy << (gen - 1) * 10;
    }

    constexpr inline Substrategy extract_substrategy(Strategy strategy, MoveGeneration::MoveGenType gen)
    {
        return (strategy >> (gen - 1) * 10) & SUBSTRATEGY_MASK;
    }

    // Complex strategies
    constexpr Strategy SIMPLE_ORDERING = 0;
    constexpr Strategy STANDARD_ORDERING = 
        make_strategy(POSITIVE_SEE | ZERO_SEE, MoveGeneration::CAPTURE) |
        make_strategy(POSITIVE_SEE | ZERO_SEE, MoveGeneration::CHECK_EVASION);
    

    // --------------
    // Selector class
    // --------------

    class Selector
    {
    public:
        Selector(const BoardConfig* board, const Evaluation::Evaluator* evaluator = nullptr,
                 MoveGeneration::MoveGenType gen = MoveGeneration::PSEUDO_LEGAL,
                 MoveSelection::Strategy strategy = MoveSelection::SIMPLE_ORDERING,
                 bool cascade = true)
            : strategy(strategy), cascade(cascade), gen(gen),
              board(board), evaluator(evaluator), moves(), sectionBegin(moves.begin()), sectionEnd(moves.end()) { generateMoves(); }

        // Generator-style opertions
        EnhancedMove next();
        bool hasNext();

        // Sorting
        // - Indexer creates an integer (key) for given move
        void sort(const std::function<int32_t(const Move&)>& indexer, EnhancementMode mode = EnhancementMode::CUSTOM_SORTING);

        // Getters
        MoveGeneration::MoveGenType phase() const { return gen; }

        // Customizable selector behavior
        MoveSelection::Strategy strategy;
        bool cascade;
    
    private:
        // Helper functions
        void generateMoves();   // Generates moves according to current generation phase (gen)
        void resetSelection() { sectionBegin = moves.begin(); sectionEnd = moves.end(); legalityChecked = false; }
        void nextSelection() { sectionBegin = sectionEnd; sectionEnd = moves.end(); legalityChecked = true; }
        template <typename... Predicates> EnhancedMove* selectMove(Predicates... preds);

        // Board connection
        const BoardConfig* board;

        // Evaluator connection
        const Evaluation::Evaluator* evaluator;

        // Move handling logic
        MoveList moves;
        MoveGeneration::MoveGenType gen;
        EnhancedMove* sectionBegin;
        EnhancedMove* sectionEnd;
        bool legalityChecked = false;
    };
}