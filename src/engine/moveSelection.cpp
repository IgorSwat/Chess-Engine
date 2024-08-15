#include "moveSelection.h"


// ---------------------
// Move selector methods
// ---------------------

// TODO: It's possible to speed up the function by creating overloaded version without 'pred' parameter for default usages
template <bool reselection, typename Pred>
Move MoveSelector::selectMove(Pred pred)
{
    while (sectionBegin != sectionEnd) {
        Move move = *sectionBegin;
        if constexpr (reselection) {
            if (!board->legalityCheckLight(move)) {     // Omit move as it's not legal and won't be processed in the future
                sectionBegin++;
                continue;
            }
        }
        if (!pred(move)) {                 // Swap the move (partition step) with another not processed yet. The processed move may be used later.
            sectionEnd--;
            if (sectionBegin != sectionEnd)
                std::swap(*sectionBegin, *sectionEnd);
        }
        else {                             // Move is legal and meets the assumptions
            sectionBegin++;
            return move;
        }
    }

    return Move::null();
}

template <GenerationStrategy genStrategy, SelectionStrategy selStrategy>
Move MoveSelector::selectNext()
{
    Move move;

    // No ordering, just get the first legal move
    if constexpr (selStrategy == SelectionStrategy::SIMPLE) {
        move = selectMove<false>([](const Move& move) -> bool {return true;});
    }

    // Good captures -> average captures -> bad captures
    if constexpr (selStrategy == SelectionStrategy::STANDARD_ORDERING) {
        if (currGenType != MoveGeneration::QUIET_CHECK && currGenType != MoveGeneration::QUIET) {
            switch (stage) {
                case 1:
                    move = selectMove<false>([this](Move& move) -> bool {return SEE::evaluate(board, move) > 0;});
                    if (move == Move::null()) {
                        nextSection();
                        return selectNext<genStrategy, selStrategy>();
                    }
                    break;
                case 2:
                    move = selectMove<true>([this](const Move& move) -> bool {return move.see >= 0;});   // We assume that SEE is already calculated
                    if (move == Move::null()) {
                        nextSection();
                        return selectNext<genStrategy, selStrategy>();
                    }
                    break;
                default:
                    move = selectMove<true>([](const Move& move) -> bool {return true;});
                    break;
            }
        }
        else
            move = selectMove<false>([](const Move& move) -> bool {return true;});
    }

    // Handle edge case - end of legal moves
    if (move == Move::null()) {
        if constexpr (genStrategy == GenerationStrategy::CASCADE) {
            if (currGenType == MoveGeneration::CAPTURE) {
                generateMoves<MoveGeneration::QUIET_CHECK>();
                return selectNext<genStrategy, selStrategy>();
            }
            if (currGenType == MoveGeneration::QUIET_CHECK) {
                generateMoves<MoveGeneration::QUIET>();
                return selectNext<genStrategy, selStrategy>();
            }
        }
    }
    
    return move;
}

// Declaration of all usages
template Move MoveSelector::selectNext<GenerationStrategy::STRICT, SelectionStrategy::SIMPLE>();
template Move MoveSelector::selectNext<GenerationStrategy::STRICT, SelectionStrategy::STANDARD_ORDERING>();
template Move MoveSelector::selectNext<GenerationStrategy::CASCADE, SelectionStrategy::SIMPLE>();
template Move MoveSelector::selectNext<GenerationStrategy::CASCADE, SelectionStrategy::STANDARD_ORDERING>();