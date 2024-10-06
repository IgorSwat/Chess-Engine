#include "moveSelection.h"


// ---------------------
// Move selector methods
// ---------------------

template <bool reselection>
Move MoveSelector::selectMove()
{
    while (sectionBegin != sectionEnd) {
        Move move = *sectionBegin;
        sectionBegin++;

        if constexpr (!reselection) {
            if (board->legalityCheckLight(move))
                return move;
        }
        else
            return move;
    }

    return Move::null();
}

template <bool reselection, typename Pred>
Move MoveSelector::selectMove(Pred pred)
{
    while (sectionBegin != sectionEnd) {
        Move move = *sectionBegin;
        if constexpr (!reselection) {
            if (!board->legalityCheckLight(move)) {
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

Move MoveSelector::selectNext(GenerationStrategy genStrategy, SelectionStrategy selStrategy)
{
    Move move;

    while (true) {
        // No ordering, just get the first legal move
        if (selStrategy == SelectionStrategy::SIMPLE) {
            move = selectMove<false>();
        }

        // Good captures -> average captures -> bad captures
        else if (selStrategy == SelectionStrategy::STANDARD_ORDERING) {
            if (currGenType != MoveGeneration::QUIET_CHECK && currGenType != MoveGeneration::QUIET) {
                switch (stage) {
                    case 1:
                        move = selectMove<false>([this](Move& move) -> bool {return SEE::evaluate(board, move) > 0;});
                        if (move == Move::null()) {
                            nextSection();
                            continue;
                        }
                        break;
                    case 2:
                        move = selectMove<true>([this](const Move& move) -> bool {return move.see >= 0;});   // We assume that SEE is already calculated
                        if (move == Move::null()) {
                            nextSection();
                            continue;
                        }
                        break;
                    default:
                        move = selectMove<true>();
                        break;
                }
            }
            else
                move = selectMove<false>();
        }

        else if (selStrategy == SelectionStrategy::IMPROVED_ORDERING) {
            if (currGenType != MoveGeneration::QUIET_CHECK && currGenType != MoveGeneration::QUIET) {
                switch (stage) {
                    case 1:
                        move = selectMove<false>([this](Move& move) -> bool {return SEE::evaluate(board, move) > 0;});
                        if (move == Move::null()) {
                            nextSection();
                            continue;
                        }
                        break;
                    case 2:
                        move = selectMove<true>([this](const Move& move) -> bool {
                            return move.see >= 0 && (move.to() == dFrom || move.from() == dTo);
                        });
                        if (move == Move::null()) {
                            nextSection();
                            continue;
                        }
                        break;
                    default:
                        move = selectMove<true>();
                        break;
                }
            }
            else {
                switch (stage) {
                    case 1:
                        move = selectMove<false>([this](const Move& move) -> bool { return move.from() == dTo; });
                        if (move == Move::null()) {
                            nextSection();
                            continue;
                        }
                        break;
                    default:
                        move = selectMove<true>();
                }
            }
            if (move == Move::null())
                stage = 1;
        }

        // Handle edge case - end of legal moves
        if (move == Move::null()) {
            if (genStrategy == GenerationStrategy::CASCADE) {
                if (currGenType == MoveGeneration::CAPTURE) {
                    generateMoves<MoveGeneration::QUIET_CHECK>();
                    continue;
                }
                if (currGenType == MoveGeneration::QUIET_CHECK) {
                    generateMoves<MoveGeneration::QUIET>();
                    continue;
                }
            }
        }

        break;
    }

    return move;
}