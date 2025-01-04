#include "moveSelection.h"
#include <algorithm>
#include <iterator>


namespace MoveSelection {

    // --------------------------------------------
    // Selector methods - generator-style opertions
    // --------------------------------------------

    EnhancedMove Selector::next(bool useStrategy)
    {
        while (true) {
            // Check if current bucket contains more moves
            if (buckets[currentBucket]) {
                const std::uint8_t index = Bitboards::pop_lsb(buckets[currentBucket]);
                const EnhancedMove& move = moves[index + currentBatch * 64];

                // Check for the case where move is excluded after some selection already being done
                if (!isExcluded(move)) {
                    lastMoveID = index;
                    return move;
                }
                else
                    continue;
            }
            
            // If no moves are present in a bucket, try to find appropriate move in the remaining portion of current batch
            // If you encounter moves from any other bucket, save them to speed up future selection
            while (nextMove != sectionEnd) {
                EnhancedMove& move = *nextMove;

                // Ignore illegal and excluded moves
                if (!board->isLegal(move) || isExcluded(move)) {
                    nextMove++;
                    continue;
                }

                std::uint8_t bucketID = useStrategy ? strategy.evaluate(gen, move) : 0;
                
                if (bucketID == currentBucket) {
                    lastMoveID = std::distance(sectionBegin, nextMove);
                    return *nextMove++;
                }
                else
                    buckets[bucketID] |= 0x1ULL << std::distance(sectionBegin, nextMove++);
            }

            // If we reach end of the batch without returning appropriate move, then the current bucket is empty and we should move on
            currentBucket++;

            // If we checked all the buckets, then it's time to change the batch
            if (currentBucket == MAX_NO_BUCKETS)
                nextSection();
            
            // If batch is empty, then there are no more moves to check
            // Therefore we should either continue with next phase (cascade selection) or break and return null move (strict selection)
            if (sectionBegin == sectionEnd) {
                gen = gen == MoveGeneration::CAPTURE ? MoveGeneration::QUIET_CHECK :
                      gen == MoveGeneration::QUIET_CHECK ? MoveGeneration::QUIET : MoveGeneration::NONE;
                
                if (cascade && gen != MoveGeneration::NONE)
                    generateMoves();
                else
                    break;
            }
        }

        return Move::null();
    }

    bool Selector::hasNext(bool test)
    {
        std::uint8_t tmp_bucket = currentBucket;

        currentBucket = 0;
        EnhancedMove move = next(false);    // Equivalent to using SIMPLE_ORDERING

        if (move == Move::null())
            return false;

        // Since we use SIMPLE_ORDERING, we know that increment of nextMove was the last operation performed inside next() function
        // Therefore, simple decrementation of the pointer ensures that move selection process is still valid
        nextMove--;
        currentBucket = tmp_bucket;

        return true;
    }


    // -------------------------------------
    // Selector methods - sorting operations
    // -------------------------------------

    void Selector::sort(const std::function<int32_t(const Move&)>& indexer, EnhancementMode mode)
    {
        // Perform sorting in place on remaining moves range

        // Calculate indices
        std::for_each(nextMove, moves.end(), 
                      [&indexer, mode](EnhancedMove& move) -> void { move.enhance(mode, indexer(move)); });
                      
        // Sort - descending order
        std::sort(nextMove, moves.end(),
                  [](EnhancedMove& a, EnhancedMove& b) -> bool { return a.key() > b.key(); });
    }


    // ---------------------------
    // Selector methods - strategy 
    // ---------------------------

    std::uint8_t Selector::Strategy::evaluate(MoveGeneration::Phase phase, EnhancedMove& move) const
    {
        std::uint8_t bucket = 0;

        for (const Predicate& pred : decisionLists[phase]) {
            if ((selector->*pred)(move))
                break;
            bucket++;
        }

        return bucket;
    }


    // -----------------------------------
    // Selector methods - helper functions
    // -----------------------------------

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
        resetSection();
    }

    void Selector::resetSection()
    {
        sectionBegin = moves.begin();
        sectionEnd = std::min(sectionBegin + 64, moves.end());
        nextMove = sectionBegin;

        currentBatch = 0;
        currentBucket = 0;
        std::fill(buckets, buckets + MAX_NO_BUCKETS, 0ULL);
    }

    void Selector::nextSection()
    {
        sectionBegin = sectionEnd;
        sectionEnd = std::min(sectionEnd + 64, moves.end());
        nextMove = sectionBegin;

        currentBatch++;
        currentBucket = 0;
        std::fill(buckets, buckets + MAX_NO_BUCKETS, 0ULL);
    }


    // ------------------------------------
    // Predefined selection strategy makers
    // ------------------------------------

    void simple_ordering(Selector& selector)
    {
        selector.strategy.clearRules();
    }

    void standard_ordering(Selector& selector)
    {
        selector.strategy.clearRules();

        selector.strategy.addRule(MoveGeneration::CAPTURE, &Selector::positiveSee);
        selector.strategy.addRule(MoveGeneration::CAPTURE, &Selector::neutralSee);

        selector.strategy.addRule(MoveGeneration::CHECK_EVASION, &Selector::positiveSee);
        selector.strategy.addRule(MoveGeneration::CHECK_EVASION, &Selector::neutralSee);
    }

    void improved_ordering(Selector& selector)
    {
        selector.strategy.clearRules();

        selector.strategy.addRule(MoveGeneration::CAPTURE, &Selector::positiveSee);
        selector.strategy.addRule(MoveGeneration::CAPTURE, &Selector::neutralSee_threatEvasion);
        selector.strategy.addRule(MoveGeneration::CAPTURE, &Selector::neutralSee);
        selector.strategy.addRule(MoveGeneration::CAPTURE, &Selector::threatCreation);

        selector.strategy.addRule(MoveGeneration::CHECK_EVASION, &Selector::positiveSee);
        selector.strategy.addRule(MoveGeneration::CHECK_EVASION, &Selector::neutralSee_threatEvasion);
        selector.strategy.addRule(MoveGeneration::CHECK_EVASION, &Selector::neutralSee);

        selector.strategy.addRule(MoveGeneration::QUIET_CHECK, &Selector::safe_threatCreation);
        selector.strategy.addRule(MoveGeneration::QUIET_CHECK, &Selector::safe_threatEvasion);
        selector.strategy.addRule(MoveGeneration::QUIET_CHECK, &Selector::safe);
        selector.strategy.addRule(MoveGeneration::QUIET_CHECK, &Selector::threatCreation);

        selector.strategy.addRule(MoveGeneration::QUIET, &Selector::safe_threatCreation);
        selector.strategy.addRule(MoveGeneration::QUIET, &Selector::safe_threatEvasion);
        selector.strategy.addRule(MoveGeneration::QUIET, &Selector::safe);
        selector.strategy.addRule(MoveGeneration::QUIET, &Selector::threatCreation);
    }

}