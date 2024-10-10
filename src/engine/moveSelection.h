#pragma once

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

    // Define hooks for other flags
    // We assume following offsets: QUIET = 0, QUIET_CHECK = 10, CAPTURE = 20, CHECK_EVASION = 30, LEGAL = 40, PSEUDO_LEGAL = 50
    // Do calculate phase offset we can use offset = (phase - 1) * 10 formula
    constexpr int QUIET_OFFSET = 0;
    constexpr int QUIET_CHECK_OFFSET = QUIET_OFFSET + 10;
    constexpr int CAPTURE_OFFSET = QUIET_CHECK_OFFSET + 10;
    constexpr int CHECK_EVASION_OFFSET = CAPTURE_OFFSET + 10;
    constexpr int LEGAL_OFFSET = CHECK_EVASION_OFFSET + 10;
    constexpr int PSEUDO_LEGAL_OFFSET = LEGAL_OFFSET + 10;

    // Local strategies
    constexpr Strategy STRATEGY_BASE = 0x1;
    constexpr Strategy POSITIVE_SEE = STRATEGY_BASE;
    constexpr Strategy MOVE_TO = STRATEGY_BASE << 1;
    constexpr Strategy MOVE_FROM = STRATEGY_BASE << 2;
    constexpr Strategy ZERO_SEE = STRATEGY_BASE << 3;

    // Complex strategies
    constexpr Strategy SIMPLE_ORDERING = 0;
    constexpr Strategy STANDARD_ORDERING = 
        (POSITIVE_SEE | ZERO_SEE) << CAPTURE_OFFSET |
        (POSITIVE_SEE | ZERO_SEE) << CHECK_EVASION_OFFSET;
}


// -------------------
// Move selector class
// -------------------

class MoveSelector;
namespace MoveSelection { template <typename Functor> void sort_moves(MoveSelector&, Functor); }

class MoveSelector
{
public:
    MoveSelector(BoardConfig* board) : board(board), moves(), sectionBegin(moves.begin()), sectionEnd(moves.end()) {}

    // Selector setup
    void setBoard(BoardConfig* board);                                  // Changes the connected board, resets moves
    template <MoveGeneration::MoveGenType gen> void generateMoves();    // Generates new moves (acc. to board), resets selection strategy
    void setStrategy(MoveSelection::Strategy);                          // Sets new selection strategy

    // Move selection
    Move selectNext(bool cascade);
    bool hasMoves();            // Check for any legal move existance

    // Helper functions
    void sortCaptures();    // A specialized, in-place sort by SEE value
    template <typename Functor> friend void MoveSelection::sort_moves(MoveSelector& selector, Functor func);

    MoveGeneration::MoveGenType currGenType = MoveGeneration::NONE;
    MoveSelection::Strategy strategy = MoveSelection::SIMPLE_ORDERING;

    // Objects of interest - for different strategies like select moves from given square, select promotions, etc.
    Square dFrom = INVALID_SQUARE;
    Square dTo = INVALID_SQUARE;
    
private:
    // 'reselection' parameter decides whether we already checked some move (which means we do not need to check it's legality again)
    Move selectMove(bool reselection);                                      // No filtering
    template <typename Pred> Move selectMove(bool reselection, Pred pred);  // Additional filtering
    void nextSection();

    void resetSelection() { sectionBegin = moves.begin(); sectionEnd = moves.end(); legalityChecked = false; }
    void nextSelection() { sectionBegin = sectionEnd; sectionEnd = moves.end(); legalityChecked = true; }

    BoardConfig* board;

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
    strategy = MoveSelection::SIMPLE_ORDERING;
}

inline void MoveSelector::setStrategy(MoveSelection::Strategy strategy)
{
    this->strategy = strategy;
}

// Warning - use this one carefully!
inline bool MoveSelector::hasMoves()
{
    MoveSelection::Strategy tmp = this->strategy;
    setStrategy(MoveSelection::SIMPLE_ORDERING);
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