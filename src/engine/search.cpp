#include "search.h"
#include "ttable.h"
#include <cassert>


namespace Search {

    // ------------------------------
    // Search - crawlers - search API
    // ------------------------------

    Score Crawler::search(Depth depth)
    {
        // Reset node counters
        non_leaf_nodes = leaf_nodes = qs_nodes = 0;

        // Prepare search stack
        // Reset all the search stack data, that is not being reset after every make & unmake of move
        for (int i = 0; i < MAX_TOTAL_SEARCH_DEPTH + 1; i++)
            std::fill(m_search_stack[i].killers, m_search_stack[i].killers + NO_KILLERS, Moves::null);

        // Push the stack to it's first real entry (which is not guard)
        // - We can achieve that by pushing it will null move, which then will be discarded with static changes
        // - This just ensures that next ply data is valid without additional code
        m_sstop = m_search_stack;         // First we point to the guard
        make_move(Moves::null, true);     // Then we move from guard to first real entry (without changing board state)

        // Decide on whether to use LMR heuristic in current search or not
        m_use_lmr = depth > 5;

        // Finally, we can step into the main search routine
        // - Alpha must be initialized with -infinity (in this case, -MAX_EVAL) and beta with +infinity for algorithm to work properly
        // - Also, let's prevent user from exceeding max depth
        Score result = search<ROOT_NODE>(-Evaluation::MAX_EVAL, Evaluation::MAX_EVAL,
                                         std::min(depth, MAX_SEARCH_DEPTH));
        
        return result;
    }


    // ---------------------------------------------------
    // Search - crawlers - search components - main search
    // ---------------------------------------------------

    template <Node node>
    Score Crawler::search(Score alpha, Score beta, Depth depth)
    {
        // TEST & DEBUG
        // Update node counters
        if (depth <= 0)
            leaf_nodes++;
        else
            non_leaf_nodes++;

        // Save current search depth
        m_sstop->depth = depth;

        // Step 1 - detect draws within game rules
        // ---------------------------------------
        // - Possible draws include draw by 50-move rule or by 3-fold repetitions
        // - We also consider 2-fold repetition as a draw if repetition happened in the same search tree

        // 50-move rule
        if (m_virtual_board.halfmoves_c() >= 100)
            return 0;
        
        // Repetitions
        auto [repetitions, repetition_dist] = m_virtual_board.repetitions();

        // We return drawn score if:
        // a) 50-move rule occured
        // b) 3-fold repetition occured
        // c) 2-fold repetition in current search tree occured
        if (repetitions == 3 || repetitions == 2 && m_sstop->ply >= repetition_dist)
            return 0;


        // Step 2 - transposition table probe
        // ----------------------------------
        // - Transposition table can produce a full cut-off if specyfic conditions are met
        // - In other case, we use best move from transposition table entry to improve move ordering

        auto tt_entry = m_ttable->probe(m_virtual_board.hash(), m_virtual_board.pieces());

        Score tt_score = Evaluation::NO_EVAL;
        EMove tt_move = Moves::null;

        if (tt_entry) {
            // DEBUG
            // Test if transposition table move is fine (helps detecting hash collisions)
            if (tt_entry->best_move != Moves::null && !m_virtual_board.is_legal_f(tt_entry->best_move)) {
                std::cout << "Illegal transposition table move!\n";
                goto next_step;
            }

            tt_score = tt_entry->score;
            tt_move = tt_entry->best_move;

            m_sstop->best_move = tt_move;

            // Cut-off condition
            // - TERMINAL_NODE always produces a cut-off, since mate or stealmate is unavoidable
            // - PV_NODE produces a cut-off if entry depth is not less than current search depth
            // - CUT_NODE requires additional condition of score being not less than current beta
            // - ALL_NODE requires additional condition of score being less than current alpha
            if (tt_entry->node_type == TERMINAL_NODE ||
                tt_entry->depth >= depth && 
                (tt_entry->node_type == PV_NODE || 
                 tt_entry->node_type == CUT_NODE && tt_entry->score >= beta ||
                 tt_entry->node_type == ALL_NODE && tt_entry->score < alpha))
            {
                // Additional protection against repetition cycles
                // - Repetition cycle is a situation, where transposition table in position A points to position B, and in B to A
                // - This can happen in case of different searches performed from both positions A and B
                if (m_virtual_board.irreversible_distance() < 4 || tt_move == Moves::null)
                    return tt_score;

                // Go into the next position and count repetitions
                // NOTE: Since this is not search, we do not need to call eternal make_move() or undo_move() functions
                m_virtual_board.make_move(tt_move);
                uint16_t next_repetitions = m_virtual_board.repetitions().first;
                m_virtual_board.undo_move();

                // If either one of the positions is not repeated, we can safely return transposition table score
                if (repetitions < 2 || next_repetitions < 2)
                    return tt_score;
                
                // In other case, we assume that move pointed out by transposition table leads to a draw
                tt_score = 0;
            }
            // If cut-off is not possible, then try transposition table move and perform standard search
            else if (depth > 0 && tt_move != Moves::null) {
                make_move(tt_move);
                tt_score = -search<PV_NODE>(-beta, -alpha, depth - 1);
                undo_move();
            }

            // Case 1 - best score reached
            if (tt_score > m_sstop->score) {
                m_sstop->score = tt_score;
                m_sstop->best_move = tt_move;
                if (tt_score > alpha) {
                    alpha = tt_score;
                    m_sstop->node = PV_NODE;
                }
            }

            // Case 2 - beta cut-off
            if (tt_score != Evaluation::NO_EVAL && tt_score >= beta) {
                m_sstop->node = CUT_NODE;

                m_ttable->set({
                    m_virtual_board.hash(),
                    m_virtual_board.pieces(),
                    m_search_stack[1].age,              // Root age
                    m_virtual_board.halfmoves_p(),      // Current age
                    depth,
                    CUT_NODE,
                    tt_score,  // score
                    tt_move,
                    tt_entry->static_eval
                });

                // Killer heuristic update
                if (tt_move.is_quiet() || m_virtual_board.see(tt_move) <= 0)
                    m_sstop->add_killer(tt_move);

                // History heuristic update
                // - Since move is best at current node, we update it's score with 1 (MAX_HISTORY_SCORE)
                // - There are no other moves which would have been tried before tt_move, so we can update only for tt_move
                m_history->update(m_virtual_board, tt_move, 
                                  HISTORY_CUT_FACTOR, m_search_stack[1].depth, depth, m_sstop->ply, HISTORY_MAX_SCORE);

                return tt_score;
            }

            m_sstop->move_idx++;

            // Get evaluations from transposition table enrty
            m_sstop->static_eval = tt_entry->static_eval;
            m_sstop->eval = tt_entry->score;
        }

        // Step 2.5 - switch to a quiscence search
        // ---------------------------------------
        // - This step happens only at leaf nodes of mmain search tree (with depth <= 0)
        // - After exhausting all the search depth it's time to perform quiescence and return result

        next_step:
        if (depth <= 0)
            return quiescence<ROOT_NODE>(alpha, beta, MAX_QUIESCENCE_DEPTH);
        
        // Step 3 - static evaluation of the position
        // ------------------------------------------
        // - Performing static eval, despite being significant computational overhead, is necessary for some search heuristics like NMP
        // - Using additional heuristics and improing search quiality neglates computetional overhead
        // - We can use value extracted from transposition table if there is one available

        if (m_sstop->static_eval == Evaluation::NO_EVAL)
            m_sstop->static_eval = evaluate();
        
        // Step 4 - NMP (Null Move Pruning) heuristic
        // ------------------------------------------
        // - Check the conditions for NMP
        // - If conditions are met and cut-node is likely to happen in currect ply, perform search at reduced depth
        // - If obtained score exceeds beta, perform a cut-off

        // ...

        // Step 5 - move generation & mate / stealmate detection
        // -----------------------------------------------------
        // - Mate occurs when side to move has no legal moves while being in check
        // - Stealmate occurs when side to move has no legal moves while not being in check

        // For better move ordering, we should always start with captures unless side to move is in check
        MoveGeneration::Mode mode = m_virtual_board.in_check() ? MoveGeneration::CHECK_EVASION : MoveGeneration::PSEUDO_LEGAL;

        MoveOrdering::Selector move_selector(&m_virtual_board, mode);

        // Detect mate & stealmate
        if (!move_selector.has_next()) {
            // For mate we use score = infinity (certain win), for stealmate we use score = 0 (draw)
            // - We use different mate score for mate in 1, mate in 2, etc.
            // - Quickest path to mate gets the highest score, which allows engine to converge into mate
            Score score = m_virtual_board.in_check() ? -Evaluation::MAX_EVAL + m_sstop->ply : 0;

            m_ttable->set({
                m_virtual_board.hash(),
                m_virtual_board.pieces(),
                m_search_stack[1].age,              // Root age
                m_virtual_board.halfmoves_p(),      // Current age
                depth,
                TERMINAL_NODE,
                score,
                Moves::null,
                m_sstop->static_eval
            });

            return score;
        }

        // Step 6 - move ordering strategies
        // ---------------------------------
        // - History heuristic application
        // - Order: winning captures (SEE > 0) first, then all moves ordered by history scores

        MoveOrdering::sort(move_selector, [this](const Move& move) -> int32_t {
            if (!move.is_quiet()) {
                int32_t see = this->m_virtual_board.see(move);
                if (see > 0)    // Prioritize winning captures and promotions
                    return HISTORY_MAX_SCORE + see;
            }
            
            return this->m_history->score(this->m_virtual_board, move);
        });

        // Step 7 - main search loop
        // -------------------------
        // - Iterates over set of legal moves, recursively going down with depth

        // Some flow control variables
        EMove move;
        int next_killer = 0;

        // History heuristic variables
        // - List of moves to remember every move tried and apply appropriate score after calculating best move and best score
        Moves::List<EMove, 64> moves_tried;

        // Before entering main loop, exclude transposition table suggestion from move selection
        // - Another occurance of previously analyzed move would have a bad effect for LMR heuristic
        if (tt_move != Moves::null) {
            move_selector.exclude(tt_move);

            tt_move.enhance(Moves::Enhancement::PURE_SEARCH_SCORE, tt_score);
            moves_tried.push_back(tt_move);
        }
        
        // Enter main loop
        // - We use infinite loop with continue/break controls for more flexibility
        while (true) {

            // Since we use only PSEUDO_LEGAL and CHECK_EVASION, we can limit phase change to STRICT
            // - No additional selection (at least for now)
            move = move_selector.next(MoveOrdering::Selector::STRICT, false);

            if (move == Moves::null)
                break;
            
            m_sstop->move_idx++;

            // Step 8 - killer heuristic
            // -------------------------
            // - Killer heuristic focuses on ordering high moves that caused cut-offs in sibling nodes
            // - Only quiet moves and bad (SEE < 0) captures can be considered as killers, since good and equal captures are already ordered high
            // - Killer moves are ordered right after winning (SEE > 0) captures

            Move killer;

            // Consider killer moves only after next ordered move is no longer a winning capture
            if (next_killer < NO_KILLERS && (move.is_quiet() || m_virtual_board.see(move) <= 0)) {
                killer = m_sstop->killers[next_killer];

                // Check legality of the killer move (full check, since killer might not even be pseudolegal in current position)
                if (killer != Moves::null && killer != move && m_virtual_board.is_legal_f(killer)) {
                    // Save generated move for later use
                    move_selector.restore_last();

                    // Switch to the killer move
                    move = killer;

                    // Exclude killer move from selection to avoid repeating the same move later
                    move_selector.exclude(killer);
                }

                next_killer++;
            }

            // Step 9 - futility pruning heuristic
            // -----------------------------------
            // - Futility pruning discards quiet moves near leaf nodes with no perspective of raising alpha
            // - Relies on arbitrary selected parameters which decide whether given position has enough potential to raise alpha

            // We use different evaluation margin for depth 1 and depth 2
            Evaluation::Eval margin = depth == 1 ? FUTILITY_MARGIN_I : FUTILITY_MARGIN_II;

            if (depth <= 2 &&
                m_sstop->static_eval + margin < alpha &&
                !m_virtual_board.in_check() && move.is_quiet() && !m_virtual_board.is_check(move))
            {
                continue;
            }

            // Step 10 - late move reductions heuristic
            // ----------------------------------------
            // - Reduce low ordered (most likely unpromising) moves in depth
            // - Each move type (captures, checks, quiet) has it's individual reduction factor
            // - Most important search heuristic, which allows to almost double effective search depth in given time frame

            Depth reduction = 1;

            if (m_use_lmr && depth > 2) {
                // Select appropriate reduction factor according to move category (quiet vs capture/promotion vs check)
                double factor = m_virtual_board.is_check(move) ? LMR_CHECK_FACTOR :
                                !move.is_quiet()               ? LMR_CAPTURE_FACTOR :
                                                                 LMR_QUIET_FACTOR;

                reduction += std::min(Depth(factor * std::log2(std::max(m_sstop->move_idx - LMR_ACTIVATION_OFFSET, 1)) * (depth - 2) / 8),
                                      Depth((depth - 2) / 2));
            }

            // Step 11 - search descent
            // ------------------------
            // - Try every move with verification search in case of LMR

            // DEBUG
            // Test if transposition table move is fine (helps detecting hash collisions)
            if (move != Moves::null && !m_virtual_board.is_legal_f(move))
                std::cout << "Illegal move in main search loop!\n";

            // Make move and search further
            make_move(move);
            Score score = -search<PV_NODE>(-beta, -alpha, depth - reduction);
            undo_move();

            // Verification search
            // - Used in context of LMR heurstic - we perform additional, full depth search if some reduced move exceeds alpha
            if (reduction > 1 && score > alpha) {
                make_move(move);
                score = -search<PV_NODE>(-beta, -alpha, depth - 1);
                undo_move();
            }

            // Save move with it's score
            move.enhance(Moves::Enhancement::PURE_SEARCH_SCORE, score);
            moves_tried.push_back(move);

            // Case 1 - best score reached
            if (score > m_sstop->score) {
                m_sstop->score = score;
                m_sstop->best_move = move;
                if (score > alpha) {
                    alpha = score;
                    m_sstop->node = PV_NODE;
                }
            }

            // Case 2 - beta cut-off
            if (score >= beta) {
                m_sstop->node = CUT_NODE;

                m_ttable->set({
                    m_virtual_board.hash(),
                    m_virtual_board.pieces(),
                    m_search_stack[1].age,              // Root age
                    m_virtual_board.halfmoves_p(),      // Current age
                    depth,
                    CUT_NODE,
                    score,
                    move,
                    m_sstop->static_eval
                });

                // Killer heuristic upate
                if (move.is_quiet() || m_virtual_board.see(move) <= 0)
                    m_sstop->add_killer(move);
                
                // History heuristic update
                // - Update current move (best) and all the previous ones (not the best)
                for (const EMove& move_tried : moves_tried) {
                    Score move_score = move_tried.score().value();
                    Score best_score = m_sstop->score;
                    Score best_score_normalized = std::abs(best_score) + HISTORY_EVAL_NORMALIZATION;

                    int64_t R = std::clamp(best_score_normalized - best_score + move_score, 0, best_score_normalized);
                    R = History::Score(R * R * HISTORY_MAX_SCORE / (best_score_normalized * best_score_normalized));

                    // TEST
                    assert(R <= HISTORY_MAX_SCORE);
                    assert(R >= 0);

                    m_history->update(m_virtual_board, move_tried, 
                                      HISTORY_CUT_FACTOR, m_search_stack[1].depth, depth, m_sstop->ply, R);
                }

                return score;
            }
        }

        // Step 12 - final result registration
        // -----------------------------------
        // - We reach this fragment if no beta cut-off or ny other cut-off occured during main search

        // Save results to transposition table
        m_ttable->set({
            m_virtual_board.hash(),
            m_virtual_board.pieces(),
            m_search_stack[1].age,              // Root age
            m_virtual_board.halfmoves_p(),      // Current age
            depth,
            m_sstop->node,
            m_sstop->score == -Evaluation::MAX_EVAL ? m_sstop->static_eval : m_sstop->score,
            m_sstop->best_move,
            m_sstop->static_eval
        });

        // Update history tables
        if (m_sstop->score != -Evaluation::MAX_EVAL) {
            for (const EMove& move_tried : moves_tried) {
                Score score = move_tried.score().value();
                Score best_score = m_sstop->score;
                Score best_score_normalized = std::abs(best_score) + HISTORY_EVAL_NORMALIZATION;

                int64_t R = std::clamp(best_score_normalized - best_score + score, 0, best_score_normalized);
                R = History::Score(R * R * HISTORY_MAX_SCORE / (best_score_normalized * best_score_normalized));

                // TEST
                assert(R <= HISTORY_MAX_SCORE);
                assert(R >= 0);

                m_history->update(m_virtual_board, move_tried, m_sstop->node == PV_NODE ? HISTORY_PV_FACTOR : HISTORY_ALL_FACTOR,
                                  m_search_stack[1].depth, depth, m_sstop->ply, R);
            }
        }
        
        return m_sstop->score == -Evaluation::MAX_EVAL ? m_sstop->static_eval : m_sstop->score;
    }


    // ---------------------------------------------------------
    // Search - crawlers - search components - quiescence search
    // ---------------------------------------------------------

    template <Node node>
    Score Crawler::quiescence(Score alpha, Score beta, Depth depth)
    {
        // Update node counters
        qs_nodes++;

        // Step 1 - transposition table probe
        // ----------------------------------
        // - In quiescence transposition table is used only for a potential cut-off
        // - No need to probe transposition table in root node, because it is already done in main search function

        if constexpr (node != ROOT_NODE) {
            auto tt_entry = m_ttable->probe(m_virtual_board.hash(), m_virtual_board.pieces());

            if (tt_entry && (is_pv(tt_entry->node_type) ||
                             tt_entry->node_type == CUT_NODE && tt_entry->score >= beta ||
                             tt_entry->node_type == ALL_NODE && tt_entry->score < alpha))
            {
                return tt_entry->score;
            }
        }

        // Step 2 - move generation & mate / stealmate detection
        // -----------------------------------------------------
        // - Similar to main search function

        MoveGeneration::Mode mode = m_virtual_board.in_check() ? MoveGeneration::CHECK_EVASION : MoveGeneration::CAPTURE;

        MoveOrdering::Selector move_selector(&m_virtual_board, mode);

        // Detect mate & stealmate
        if (!move_selector.has_next()) {
            // For mate we use score = infinity (certain win), for stealmate we use score = 0 (draw)
            Score score = m_virtual_board.in_check() ? -Evaluation::MAX_EVAL + m_sstop->ply : 0;

            m_ttable->set({
                m_virtual_board.hash(),
                m_virtual_board.pieces(),
                m_search_stack[1].age,              // Root age
                m_virtual_board.halfmoves_p(),      // Current age
                0,
                TERMINAL_NODE,
                score,
                Moves::null,
                Evaluation::NO_EVAL
            });

            return score;
        }

        // Step 3 - static evaluation of the position
        // ------------------------------------------
        // - Static evaluation is used as a stand-pat score in queiscence
        // - Main factor in Delta Pruning heuristic

        m_sstop->static_eval = evaluate();

        // If maximum quiescence depth is reached, then return static eval as final result
        if (depth == 0)
            return m_sstop->static_eval;

        // Step 4 - quiescence on captures
        // -------------------------------
        // - Continue search on all captures that could potentially affect the final score (raise alpha)
        // - Delta pruning

        EMove move;

        // We omit this step if there are no captures to be made
        // - NOTE: has_next() invoked in mate/stealmate detection could change generation phase from CAPTURE to QUIET_CHECK or QUIET
        if (move_selector.phase() >= MoveGeneration::CAPTURE) {
            // Sort all the captures by SEE value
            // - This allows to reduce number of iterations in the main loop below
            MoveOrdering::sort(move_selector, [this](const Move& move) { return this->m_virtual_board.see(move); }, 
                                              Moves::Enhancement::PURE_SEE);
            
            // In this loop we try only winning captures
            while (true) {
                // NOTE: STRICT mode ensures that we do not go beyond captures in this loop
                move = move_selector.next(MoveOrdering::Selector::STRICT);

                // Break condition 1 - no more winning captures
                if (move == Moves::null || move.is_quiet() || move.see().value() <= 0)
                    break;

                // Break condition 2 - no more improving captures in terms of delta pruning
                // - NOTE: we must check all the possible replies when side to move is in check
                if (m_sstop->static_eval + move.see().value() + DELTA_MARGIN < alpha && move_selector.phase() != MoveGeneration::CHECK_EVASION)
                    break;
                
                // Make move and search further
                make_move(move);
                Score score = -quiescence<NON_PV_NODE>(-beta, -alpha, depth - 1);
                undo_move();

                // Beta cut-off
                // - NOTE: Since most of results from quiescence are relative to current alpha and beta, we do not save them to transposition table
                if (score >= beta)
                    return score;

                if (score > m_sstop->score) {
                    m_sstop->score = score;
                    if (score > alpha)
                        alpha = score;
                }
            }
        }

        // Step 5 - further quiescence
        // ---------------------------
        // - Applied if position has potential to raise alpha, but is not quiet enough
        // - Not quiet enough means there are some threats (checks, multiple attacks on valuable pieces) to deal with

        // Number of threats against side to move decides whether quiescence should be continued
        Bitboard threats = m_virtual_board.threats(m_virtual_board.side_to_move());

        // Typically a single threat can be dealt with in one move, but two separate threats usually mean loss of a material
        if (m_sstop->static_eval + EPSILON_MARGIN > alpha && Bitboards::popcount(threats) > 1) {
            // Change generation phase if reached the end of previous one
            // - This time we use FULL_CASCADE since we want to test move regardless of it's category
            if (move == Moves::null)
                move = move_selector.next(MoveOrdering::Selector::FULL_CASCADE);
            
            while (move != Moves::null) {
                PieceType ptype = type_of(m_virtual_board.on(move.to()));
                Bitboard attacks = ptype == NULL_PIECE_TYPE ? 0 :
                                   ptype == PAWN ? Pieces::pawn_attacks(~m_virtual_board.side_to_move(), move.to()) :
                                                   Pieces::piece_attacks_d(ptype, move.to(), m_virtual_board.pieces());
                
                if (m_virtual_board.in_check() ||
                    move_selector.phase() != MoveGeneration::CAPTURE && threats & move.from() ||
                    move_selector.phase() == MoveGeneration::CAPTURE && 
                    m_sstop->static_eval + EPSILON_MARGIN + move.see().value() > alpha &&
                    (threats & move.from() || attacks & threats))
                {
                    // Make move and search further
                    make_move(move);
                    Score score = -quiescence<NON_PV_NODE>(-beta, -alpha, depth - 1);
                    undo_move();

                    // Beta cut-off
                    // - NOTE: Since most of results from quiescence are relative to current alpha and beta, we do not save them to transposition table
                    if (score >= beta)
                        return score;

                    if (score > m_sstop->score) {
                        m_sstop->score = score;
                        if (score > alpha)
                            alpha = score;
                    }
                }

                move = move_selector.next(MoveOrdering::Selector::FULL_CASCADE);
            }

            return m_sstop->score;
        }

        return std::max(m_sstop->static_eval, m_sstop->score);
    }


    // --------------------------------------
    // Search - crawlers - move make & unmake
    // --------------------------------------

    void Crawler::make_move(const Move& move, bool only_stack)
    {
        // Do not continue if search stack is full
        if (m_sstop->ply == MAX_TOTAL_SEARCH_DEPTH)
            return;

        if (!only_stack) {
            // NNUE must be updated before virtual board
            m_nnue.update(m_virtual_board, move);
        
            if (move != Moves::null)
                m_virtual_board.make_move(move);
            else
                m_virtual_board.make_null_move();
        }
        
        // Push search stack
        m_sstop++;

        // Reset next ply data (there might be some info from previous visit at that ply)
        // - NOTE: we can't simply use *m_sstop = {}, because we want to reuse some data (like killers list)
        m_sstop->ply = (m_sstop - 1)->ply + 1;
        m_sstop->age = m_virtual_board.halfmoves_p();
        m_sstop->score = -Evaluation::MAX_EVAL;
        m_sstop->best_move = Moves::null;
        m_sstop->node = ALL_NODE;
        m_sstop->static_eval = Evaluation::NO_EVAL;
        m_sstop->eval = Evaluation::NO_EVAL;
        m_sstop->move_idx = 0;
    }

    void Crawler::undo_move()
    {
        // Do not continue if search stack is empty (stack top pointer points to guard)
        if (m_sstop->ply == -1)
            return;

        m_nnue.undo_state();

        if (m_virtual_board.last_move() != Moves::null)
            m_virtual_board.undo_move();
        else
            m_virtual_board.undo_null_move();
        
        // Decrement stack pointer
        m_sstop--;
    }

}