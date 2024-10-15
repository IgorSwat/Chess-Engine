#include "moveSelection.h"
#include <bitset>


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
        MoveSelection::Substrategy substrategy = MoveSelection::extract_substrategy(this->strategy, this->currGenType);

        // Adjust selection of next move according to most important flag (least significant bit)
        if (substrategy & MoveSelection::POSITIVE_SEE)
            move = selectMove(false, [this](Move& move) -> bool { return SEE::evaluate(board, move) > 0; });
        else if (substrategy & MoveSelection::ZERO_SEE) {
            move = this->seeChecked ? selectMove(this->legalityChecked, [this](const Move& move) -> bool {return move.see >= 0;}) :
                                      selectMove(this->legalityChecked, [this](const Move& move) -> bool {return SEE::evaluate(board, move) >= 0;});
        }
        else if (substrategy & MoveSelection::THREAT_EVASION) {
            move = selectMove(this->legalityChecked, [this](const Move& move) -> bool { 
                return this->evaluator->threatMap[this->board->movingSide()] & move.from();
            });
        }
        else
            move = selectMove(this->legalityChecked);

        // Checpoint 1 - selection phase adjustment
        if (move == Move::null() && substrategy) {
            if (substrategy & 0x3)
                this->seeChecked = true;

            // Remove LSB from substrategy to discard already finished selection phase
            substrategy &= (substrategy - 1);
            this->strategy &= MoveSelection::make_strategy(substrategy, this->currGenType);

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