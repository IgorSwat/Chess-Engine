#pragma once

#include "moveGeneration.h"
#include "see.h"
#include <algorithm>


// ------------------------
// Selection behavior flags
// ------------------------

enum class GenerationStrategy {
    STRICT = 1, // Selects only moves from specified MoveGenType
    CASCADE,    // Selects moves from another MoveGenType if current MoveGenType is completely used
};

// Defines strategy of selecting (filtering) moves from given MoveGenType. It's usually equivallent to imposing a predicate.
enum class SelectionStrategy {
    // SEE-dependable
    SIMPLE = 1,        // No strategy, selects every legal move in generation order
    STANDARD_ORDERING, // Selects good captures, then average captures, then bad captures, and then other moves

    // Other
    // ...
};


// -------------------
// Move selector class
// -------------------

class MoveSelector;
namespace MoveSelection { template <typename Functor> void sort_moves(MoveSelector&, Functor); }

class MoveSelector
{
public:
    MoveSelector(BoardConfig* board) : board(board), moves(), sectionBegin(moves.begin()), sectionEnd(moves.end()) {}

    // Change board
    void setBoard(BoardConfig* board);

    // Generating moves
    template <MoveGeneration::MoveGenType gen>
    void generateMoves();   // Initializes move list by generating moves 

    // Move selection
    template <GenerationStrategy genStrategy, SelectionStrategy selStrategy>
    Move selectNext();
    bool hasMoves();            // Check for any legal move existance

    // Reseting the selection
    void resetSelection();      // Reset current move set in both ends to allow another loop

    // Helper functions
    void sortCaptures();    // A specialized, in-place sort by SEE value
    template <typename Functor> friend void MoveSelection::sort_moves(MoveSelector& selector, Functor func);

    MoveGeneration::MoveGenType currGenType = MoveGeneration::NONE;
    
private:
    // 'reselection' parameter decides whether we already checked some move (which means we do not need to check it's legality again)
    template <bool reselection> Move selectMove();                          // No filtering
    template <bool reselection, typename Pred> Move selectMove(Pred pred);  // Additional filtering
    void nextSection();

    BoardConfig* board;

    // Move handling
    MoveList moves;
    Move* sectionBegin;
    Move* sectionEnd;
    int stage = 1;      // Used in the scope of one generation type

    // Objects of interest - for different strategies like select moves from given square, select promotions, etc.
    Square dFrom;
    Square dTo;
};


// ---------------------
// Move selector methods
// ---------------------

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
}

// Warning - use this one carefully!
inline bool MoveSelector::hasMoves()
{
    Move move = selectNext<GenerationStrategy::CASCADE, SelectionStrategy::SIMPLE>();   // It can change the current generation type
    sectionBegin = moves.begin();
    return move != Move::null();
}

inline void MoveSelector::resetSelection()
{
    sectionBegin = moves.begin();
    sectionEnd = moves.end();
    stage = 1;
}

inline void MoveSelector::sortCaptures()
{
    std::for_each(moves.begin(), moves.end(), [this](Move& move) -> void { SEE::evaluate(this->board, move); });
    std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) -> bool { return a.see > b.see; });   // Descending order
}

inline void MoveSelector::nextSection()
{
    sectionBegin = sectionEnd;
    sectionEnd = moves.end();
    stage++;
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