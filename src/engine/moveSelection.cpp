#include "moveSelection.h"
#include <algorithm>
#include <iterator>


namespace MoveSelection {

    // --------------------------------------------
    // Selector methods - generator-style opertions
    // --------------------------------------------

    EnhancedMove Selector::next()
    {
        while (true) {
            // Check if current bucket contains more moves
            if (buckets[currentBucket])
                return moves[Bitboards::pop_lsb(buckets[currentBucket]) + currentBatch * 64];

            // Extract substrategy for current phase
            Substrategy substrategy = extract_substrategy(strategy, gen);
            
            // If no moves are present in a bucket, try to find appropriate move in the remaining portion of current batch
            // If you encounter moves from any other bucket, save them to speed up future selection
            while (nextMove != sectionEnd) {
                EnhancedMove& move = *nextMove;

                // Check legality of the move
                if (!board->legalityCheckLight(move)) {
                    nextMove++;
                    continue;
                }

                // Linearized decision tree based on the given substrategy
                int bucketID;
                if (substrategy & POSITIVE_SEE && SEE::evaluate_save(board, move) > 0)
                    bucketID = Bitboards::popcount(substrategy & (POSITIVE_SEE - 1));
                else if (substrategy & NEUTRAL_SEE_THREAT_CREATION && SEE::evaluate_save(board, move) == 0 &&
                                                                      evaluator->isCreatingThreats(move))
                    bucketID = Bitboards::popcount(substrategy & (NEUTRAL_SEE_THREAT_CREATION - 1));
                else if (substrategy & NEUTRAL_SEE_THREAT_EVASION && SEE::evaluate_save(board, move) == 0 &&
                                                                     evaluator->isAvoidingThreats(move))
                    bucketID = Bitboards::popcount(substrategy & (NEUTRAL_SEE_THREAT_EVASION - 1));
                else if (substrategy & NEUTRAL_SEE && SEE::evaluate_save(board, move) == 0)
                    bucketID = Bitboards::popcount(substrategy & (NEUTRAL_SEE - 1));
                else if (substrategy & NEGATIVE_SEE_THREAT_CREATION && evaluator->isCreatingThreats(move))
                    bucketID = Bitboards::popcount(substrategy & (NEGATIVE_SEE_THREAT_CREATION - 1));
                else if (substrategy & NEGATIVE_SEE_THREAT_EVASION && evaluator->isAvoidingThreats(move))
                    bucketID = Bitboards::popcount(substrategy & (NEGATIVE_SEE_THREAT_EVASION - 1));
                else
                    bucketID = Bitboards::popcount(substrategy);
                
                if (bucketID == currentBucket)
                    return *nextMove++;
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

    bool Selector::hasNext()
    {
        Strategy tmp_strategy = strategy;
        std::uint8_t tmp_bucket = currentBucket;

        strategy = SIMPLE_ORDERING;
        currentBucket = 0;
        EnhancedMove move = next();

        if (move == Move::null())
            return false;

        // Since we use SIMPLE_ORDERING, we know that increment of nextMove was the last operation performed inside next() function
        // Therefore, simple decrementation of the pointer ensures that move selection process is still valid
        nextMove--;
        strategy = tmp_strategy;
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

}