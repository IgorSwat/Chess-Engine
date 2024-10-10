#include "moveSelection.h"


// ---------------------
// Move selector methods
// ---------------------

Move MoveSelector::selectMove(bool reselection)
{
    while (sectionBegin != sectionEnd) {
        Move move = *sectionBegin;
        sectionBegin++;

        if (reselection || board->legalityCheckLight(move))
            return move;
    }

    return Move::null();
}

template <typename Pred>
Move MoveSelector::selectMove(bool reselection, Pred pred)
{
    while (sectionBegin != sectionEnd) {
        Move move = *sectionBegin;

        if (!reselection && !board->legalityCheckLight(move)) {
            sectionBegin++;
            continue;
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

Move MoveSelector::selectNext(bool cascade)
{
    Move move;

    while (true) {
        // Extract bitset corresponding to current move generation phase
        int phaseOffset = (this->currGenType - 1) * 10;
        MoveSelection::Strategy flags = (this->strategy >> phaseOffset) & 0x1111111111;

        // Adjust selection of next move according to most important flag (least significant bit)
        if (flags & MoveSelection::POSITIVE_SEE)
            move = selectMove(false, [this](Move& move) -> bool { return SEE::evaluate(board, move) > 0; });
        else if (flags & MoveSelection::MOVE_TO)
            move = selectMove(this->legalityChecked, [this](const Move& move) -> bool { return move.to() == this->dTo; });
        else if (flags & MoveSelection::MOVE_FROM)
            move = selectMove(this->legalityChecked, [this](const Move& move) -> bool { return move.from() == this->dFrom; });
        else if (flags & MoveSelection::ZERO_SEE) {
            move = this->seeChecked ? selectMove(this->legalityChecked, [this](const Move& move) -> bool {return move.see >= 0;}) :
                                      selectMove(this->legalityChecked, [this](const Move& move) -> bool {return SEE::evaluate(board, move) >= 0;});
        }
        else
            move = selectMove(this->legalityChecked);

        // Checpoint 1 - selection phase adjustment
        if (move == Move::null() && flags) {
            if (flags & MoveSelection::POSITIVE_SEE)
                this->seeChecked = true;

            // Remove LSB from flags to discard already finished selection phase
            flags &= (flags - 1);
            this->strategy &= flags << phaseOffset;

            nextSelection();
            continue;
        }

        // Checkpoint 2 - generation phae adjustment (if 'cascade' option selected)
        if (move == Move::null() && cascade) {
            if (this->currGenType == MoveGeneration::CAPTURE) {
                generateMoves<MoveGeneration::QUIET_CHECK>();
                continue;
            }
            if (this->currGenType == MoveGeneration::QUIET_CHECK) {
                generateMoves<MoveGeneration::QUIET>();
                continue;
            }
        }

        break;
    }

    return move;
}