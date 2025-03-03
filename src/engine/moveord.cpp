#include "moveord.h"


namespace MoveOrdering {

    // -------------------------------
    // Selector - generator operations
    // -------------------------------

    // Extracting next element
    EMove Selector::next(Mode mode, bool use_strategy)
    {
        while (true) {
            // Check if current bucket contains more moves
            if (m_buckets[m_bucket]) {
                const std::uint32_t index = Bitboards::pop_lsb(m_buckets[m_bucket]);
                const EMove& move = m_moves[index + m_batch * 64];

                // Check for the case where move is excluded after some selection already being done
                if (!is_excluded(move)) {
                    m_last_move_id = index;
                    return move;
                }
                else
                    continue;
            }
            
            // If no moves are present in a bucket, try to find appropriate move in the remaining portion of current batch
            // If you encounter moves from any other bucket, save them to speed up future selection
            while (m_next_move != m_section_end) {
                EMove& move = *m_next_move;

                // Ignore illegal and excluded moves
                if (!m_board->is_legal_p(move) || is_excluded(move)) {
                    m_next_move++;
                    continue;
                }

                uint32_t bucketID = use_strategy ? strategy.classify(m_gen, move) : 0;
                
                if (bucketID == m_bucket) {
                    m_last_move_id = std::distance(m_section_begin, m_next_move);
                    return *m_next_move++;
                }
                else
                    m_buckets[bucketID] |= 0x1ULL << std::distance(m_section_begin, m_next_move++);
            }

            // If we reach end of the batch without returning appropriate move, then the current bucket is empty and we should move on
            m_bucket++;

            // If we checked all the buckets, then it's time to change the batch
            if (m_bucket == MAX_BUCKETS)
                set_batch(m_batch + 1);
            
            // If batch is empty, then there are no more moves to check
            // Therefore we should either continue with next phase (cascade selection) or break and return null move (strict selection)
            if (m_section_begin == m_section_end) {
                m_gen = m_gen == MoveGeneration::CAPTURE ? MoveGeneration::QUIET_CHECK :
                        m_gen == MoveGeneration::QUIET_CHECK ? MoveGeneration::QUIET : MoveGeneration::NONE;

                if (mode != STRICT && m_gen != MoveGeneration::NONE)
                    generate_moves();
                
                if (mode != FULL_CASCADE || m_gen == MoveGeneration::NONE)
                    break;
            }
        }

        return Moves::null;
    }

    // Checking whether next element can still be extracted
    // - Uses next() with revert of any changes
    // - WARNING: caution recommended when combining this with any sorting
    bool Selector::has_next()
    {
        EMove move = next(FULL_CASCADE, true);

        if (move == Moves::null)
            return false;

        restore_last();

        return true;
    }


    // -------------------
    // Selector - strategy
    // -------------------

    uint32_t Selector::Strategy::classify(MoveGeneration::Mode mode, EMove& move) const
    {
        uint32_t bucket = 0;

        for (const Predicate& pred : decisionLists[mode]) {
            if (pred(move))
                break;
            bucket++;
        }

        return bucket;
    }


    // ------------------------------------------------
    // Selector - helper functions - move pool handling
    // ------------------------------------------------

    void Selector::generate_moves()
    {
        // Start by clearing move list (generators use push_back)
        m_moves.clear();

        // NOTE: Since selector always checks legality, there is no point in handling LEGAL generation mode
        switch (m_gen) {
            case MoveGeneration::QUIET:
                MoveGeneration::generate_moves<MoveGeneration::QUIET, EMove>(*m_board, m_moves);
                break;
            case MoveGeneration::QUIET_CHECK:
                MoveGeneration::generate_moves<MoveGeneration::QUIET_CHECK, EMove>(*m_board, m_moves);
                break;
            case MoveGeneration::CAPTURE:
                MoveGeneration::generate_moves<MoveGeneration::CAPTURE, EMove>(*m_board, m_moves);
                break;
            case MoveGeneration::CHECK_EVASION:
                MoveGeneration::generate_moves<MoveGeneration::CHECK_EVASION, EMove>(*m_board, m_moves);
                break;
            default:
                MoveGeneration::generate_moves<MoveGeneration::PSEUDO_LEGAL, EMove>(*m_board, m_moves);
                break;
        }

        // Reset batch & bucket data to default (first)
        reset_batch();
    }

    void Selector::set_batch(uint8_t batch_id)
    {
        // Update move pointers
        m_section_begin = std::min(m_moves.begin() + 64 * batch_id, m_moves.end());
        m_section_end = std::min(m_section_begin + 64, m_moves.end());
        m_next_move = m_section_begin;

        // Update bucket & batch counters
        m_batch = batch_id;
        m_bucket = 0;
        std::fill(m_buckets, m_buckets + MAX_BUCKETS, 0);
    }

}