#include "moveSelection.h"
#include <algorithm>


namespace MoveSelection {

    // --------------------------------------------
    // Selector methods - generator-style opertions
    // --------------------------------------------

    EnhancedMove Selector::next()
    {
        EnhancedMove* moveptr = nullptr;

        while (true) {
            // Extract bitset corresponding to current move generation phase
            Substrategy substrategy = extract_substrategy(strategy, gen);

            // Adjust selection of next move according to most important flag (least significant bit)
            if (substrategy & POSITIVE_SEE)
                moveptr = selectMove([this](EnhancedMove& move) -> bool { return SEE::evaluate_save(board, move) > 0; });
            else if (substrategy & ZERO_SEE)
                moveptr = selectMove([this](const EnhancedMove& move) -> bool {return SEE::evaluate(board, move) >= 0;});
            else if (substrategy & THREAT_CREATION) {
                moveptr = selectMove([this](const EnhancedMove& move) -> bool {
                    return SEE::evaluate(board, move) >= 0 && this->evaluator->isCreatingThreats(move); 
                });
            }
            else if (substrategy & THREAT_EVASION) {
                moveptr = selectMove([this](const EnhancedMove& move) -> bool { 
                    return SEE::evaluate(board, move) >= 0 && this->evaluator->threatMap[this->board->movingSide()] & move.from();
                });
            }
            else
                moveptr = selectMove();
            

            // Checpoint 1 - selection phase adjustment
            if (moveptr == nullptr && substrategy) {
                // Remove LSB from substrategy to discard already finished selection phase
                substrategy &= (substrategy - 1);
                this->strategy &= make_strategy(substrategy, gen);

                nextSelection();
                continue;
            }

            // Checkpoint 2 - generation phae adjustment (if 'cascade' option selected)
            if (moveptr == nullptr && cascade) {
                gen = gen == MoveGeneration::CAPTURE ? MoveGeneration::QUIET_CHECK :
                      gen == MoveGeneration::QUIET_CHECK ? MoveGeneration::QUIET : MoveGeneration::NONE;
                
                if (gen != MoveGeneration::NONE) {
                    generateMoves();
                    continue;
                }
            }

            break;
        }

        return moveptr != nullptr ? *moveptr : Move::null();
    }

    bool Selector::hasNext()
    {
        Strategy tmp = strategy;

        strategy = SIMPLE_ORDERING;
        EnhancedMove move = next();

        resetSelection();
        strategy = tmp;

        return move != Move::null();
    }


    // // -------------------------------------
    // // Selector methods - sorting operations
    // // -------------------------------------

    void Selector::sort(const std::function<int32_t(const Move&)>& indexer, EnhancementMode mode)
    {
        // Perform sorting in place on remaining moves range

        // Calculate indices
        std::for_each(sectionBegin, sectionEnd, 
                      [&indexer, mode](EnhancedMove& move) -> void { move.enhance(mode, indexer(move)); });
                      
        // Sort - descending order
        std::sort(sectionBegin, sectionEnd,
                  [](EnhancedMove& a, EnhancedMove& b) -> bool { return a.key() > b.key(); });
    }


    // // -----------------------------------
    // // Selector methods - helper functions
    // // -----------------------------------

    void Selector::generateMoves()
    {
        auto generator = gen == MoveGeneration::QUIET ? MoveGeneration::generate_moves<MoveGeneration::QUIET> :
                         gen == MoveGeneration::QUIET_CHECK ? MoveGeneration::generate_moves<MoveGeneration::QUIET_CHECK> :
                         gen == MoveGeneration::CAPTURE ? MoveGeneration::generate_moves<MoveGeneration::CAPTURE> :
                         gen == MoveGeneration::CHECK_EVASION ? MoveGeneration::generate_moves<MoveGeneration::CHECK_EVASION> :
                         gen == MoveGeneration::PSEUDO_LEGAL ? MoveGeneration::generate_moves<MoveGeneration::PSEUDO_LEGAL> :
                                                               MoveGeneration::generate_moves<MoveGeneration::LEGAL>;

        moves.clear();
        generator(*board, moves);
        resetSelection();

        if (gen == MoveGeneration::LEGAL)
            legalityChecked = true;
    }

    template <typename... Predicates>
    EnhancedMove* Selector::selectMove(Predicates... preds)
    {
        while (sectionBegin != sectionEnd) {
            EnhancedMove& move = *sectionBegin;

            // Discard illegal moves
            if (!legalityChecked && !board->legalityCheckLight(move)) {
                sectionBegin++;
                continue;
            }

            // Check the predicates (additional trick if no predicate is specified)
            if ((true && ... && preds(move))) {
                sectionBegin++;
                return &move;
            }
            else {
                // Push the move to the end of the list (partition algorithm)
                sectionEnd--;
                if (sectionBegin != sectionEnd)
                    std::swap(*sectionBegin, *sectionEnd);
            }
        }

        // If no move found
        return nullptr;
    }

}