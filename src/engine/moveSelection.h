#pragma once

#include "moveGeneration.h"


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

class MoveSelector
{
public:
    MoveSelector(BoardConfig* board) : board(board), moves(), sectionBegin(moves.begin()), sectionEnd(moves.end()) {}

    // Generating moves
    template <MoveGeneration::MoveGenType gen>
    void generateMoves();   // Initializes move list by generating moves 

    // Selecting generated moves
    template <GenerationStrategy genStrategy, SelectionStrategy selStrategy>
    Move selectNext();
    // Reset current move set to allow another loop
    void resetSelection();

    // Change board
    void setBoard(BoardConfig* board);
    
private:
    // 'reselection' parameter decides whether we already checked some move (which means we do not need to check it's legality again)
    template <bool reselection, typename Pred>
    Move selectMove(Pred pred);
    void nextSection();

    BoardConfig* board;

    // Move handling
    MoveList moves;
    Move* sectionBegin;
    Move* sectionEnd;
    MoveGeneration::MoveGenType currGenType = MoveGeneration::NONE;
    int stage = 1;
};


// ---------------------
// Move selector methods
// ---------------------

template <MoveGeneration::MoveGenType gen>
inline void MoveSelector::generateMoves()
{
    moves.clear();
    MoveGeneration::generate_moves<gen>(*board, moves);
    currGenType = gen;
    resetSelection();
}

inline void MoveSelector::resetSelection()
{
    sectionBegin = moves.begin();
    sectionEnd = moves.end();
    stage = 1;
}

inline void MoveSelector::setBoard(BoardConfig* board)
{
    this->board = board;
    moves.clear();
}

inline void MoveSelector::nextSection()
{
    sectionBegin = sectionEnd;
    sectionEnd = moves.end();
    stage++;
}