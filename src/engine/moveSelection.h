#pragma once

#include <evaluation.h>
#include "moveGeneration.h"
#include "see.h"
#include <algorithm>


// ------------------------
// Selection behavior flags
// ------------------------

namespace MoveSelection {

    // Selection strategy is defined as 64 bit integer, with 10 consecutive bits representing strategy for given generation phase
    // For example, first 10 bits defines strategy for QUIET (moves) phase
    using Strategy = std::uint64_t;
    using Substrategy = Strategy;

    // Local strategies
    constexpr Substrategy STRATEGY_BASE = 0x1;
    constexpr Substrategy SUBSTRATEGY_MASK = 0x3ff;

    constexpr Substrategy POSITIVE_SEE = STRATEGY_BASE;
    constexpr Substrategy ZERO_SEE = STRATEGY_BASE << 1;
    constexpr Substrategy THREAT_EVASION = STRATEGY_BASE << 2;

    // Complex strategies
    constexpr inline Strategy make_strategy(Substrategy substrategy, MoveGeneration::MoveGenType gen)
    {
        return substrategy << (gen - 1) * 10;
    }

    constexpr inline Substrategy extract_substrategy(Strategy strategy, MoveGeneration::MoveGenType gen)
    {
        return (strategy >> (gen - 1) * 10) & SUBSTRATEGY_MASK;
    }

    constexpr Strategy SIMPLE_ORDERING = 0;
    constexpr Strategy STANDARD_ORDERING = 
        make_strategy(POSITIVE_SEE | ZERO_SEE, MoveGeneration::CAPTURE) |
        make_strategy(POSITIVE_SEE | ZERO_SEE, MoveGeneration::CHECK_EVASION);
}


// -------------------
// Move selector class
// -------------------

class MoveSelector;
namespace MoveSelection { template <typename Functor> void sort_moves(MoveSelector&, Functor); }

class MoveSelector
{
public:
    MoveSelector(BoardConfig* board, const Evaluation::Evaluator* evaluator = nullptr) 
        : board(board), evaluator(evaluator), moves(), sectionBegin(moves.begin()), sectionEnd(moves.end()) {}

    // Selector setup
    void setBoard(BoardConfig* board);                                  // Changes the connected board, resets moves
    template <MoveGeneration::MoveGenType gen> void generateMoves();    // Generates new moves (acc. to board), resets selection strategy

    // Move selection
    Move selectNext(bool cascade);
    bool hasMoves();            // Check for any legal move existance

    // Helper functions
    void sortCaptures();    // A specialized, in-place sort by SEE value
    template <typename Functor> friend void MoveSelection::sort_moves(MoveSelector& selector, Functor func);

    MoveGeneration::MoveGenType currGenType = MoveGeneration::NONE;
    MoveSelection::Strategy strategy = MoveSelection::SIMPLE_ORDERING;
    
private:
    // 'reselection' parameter decides whether we already checked some move (which means we do not need to check it's legality again)
    Move selectMove(bool reselection);                                      // No filtering
    template <typename Pred> Move selectMove(bool reselection, Pred pred);  // Additional filtering
    void nextSection();

    void resetSelection() { sectionBegin = moves.begin(); sectionEnd = moves.end(); legalityChecked = false; }
    void nextSelection() { sectionBegin = sectionEnd; sectionEnd = moves.end(); legalityChecked = true; }

    BoardConfig* board;
    const Evaluation::Evaluator* evaluator;

    // Move handling
    MoveList moves;
    Move* sectionBegin;
    Move* sectionEnd;
    bool legalityChecked = false;
    bool seeChecked = false;
};


// --------------------
// MoveSelector methods
// --------------------

inline void MoveSelector::setBoard(BoardConfig* board)
{
    this->board = board;
    moves.clear();
}

template <MoveGeneration::MoveGenType gen>
inline void MoveSelector::generateMoves()
{
    moves.clear();
    MoveGeneration::generate_moves<gen>(*board, moves);
    currGenType = gen;
    resetSelection();
    seeChecked = false;
}

// Warning - use this one carefully!
inline bool MoveSelector::hasMoves()
{
    MoveSelection::Strategy tmp = this->strategy;
    this->strategy = MoveSelection::SIMPLE_ORDERING;
    Move move = selectNext(true);
    resetSelection();
    this->strategy = tmp;

    return move != Move::null();
}

inline void MoveSelector::sortCaptures()
{
    std::for_each(moves.begin(), moves.end(), [this](Move& move) -> void { SEE::evaluate(this->board, move); });
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) -> bool { return a.see > b.see; });   // Descending order
}


// ---------------------------
// Other move selection issues
// ---------------------------

namespace MoveSelection {

    // ------------
    // Move sorting
    // ------------

    // WARNING - adds some additional performance overhead due to not sorting in place
    template <typename Functor>
    void sort_moves(MoveList& moveList, Functor func)
    {
        // Decorator pattern for sorting
        std::pair<Move, std::int64_t> carray[MoveList::MAX_MOVES];
        std::transform(moveList.begin(), moveList.end(), carray, 
                       [func](Move move) -> std::pair<Move, std::int64_t> { return {move, func(move)}; });
        
        std::sort(carray, carray + moveList.size(), 
                  [](const auto& a, const auto& b){ return a.second > b.second; });     // Descending order
        
        std::transform(carray, carray + moveList.size(), moveList.begin(),
                       [](const auto& p) -> Move { return p.first; });
    }

    // WARNING - adds some additional performance overhead due to not sorting in place
    template <typename Functor>
    inline void sort_moves(MoveSelector& selector, Functor func)
    {
        sort_moves(selector.moves, func);
        selector.resetSelection();
    }

}