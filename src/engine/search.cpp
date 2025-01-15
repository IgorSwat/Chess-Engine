#include "transpositionTable.h"
#include "../logic/pieces.h"
#include "../logic/randomGens.h"
#include <numeric>


namespace Search {

    // ----------------------------
    // Crawler methods - public API
    // ----------------------------

    Value Crawler::search(Depth depth)
    {
        non_leaf_nodes = leaf_nodes = qs_nodes = 0;

        if (depth == 0)
            return evaluate();

        // Before launching main search, reset all the necessary search stack data
        for (int i = 0; i < MAX_SEARCH_DEPTH + MAX_QUIESCENCE_DEPTH + 1; i++) {
            std::fill(searchStack[i].killers, searchStack[i].killers + MAX_NO_KILLERS, Move::null());
        }

        // Decide on whether to use LMR heuristic or not
        useLMR = ALLOW_LMR;
        //useLMR = ALLOW_LMR && depth > 3;  // Needs history heuristic to be fixed...

        Value score = search<ROOT_NODE, false>(-MAX_EVAL, MAX_EVAL, std::min(depth, static_cast<Depth>(MAX_SEARCH_DEPTH)));
        ssTop = searchStack;

        return score;
    }


    // -----------------------------
    // Crawler methods - main search
    // -----------------------------

    template <Node node, bool nmpAvailable>
    Value Crawler::search(Value alpha, Value beta, Depth depth)
    {
        pushStack();

        // Testing block
        // -------------

        if (depth != 0)
            non_leaf_nodes++;
        else
            leaf_nodes++;

        // Stage 1 - detect draws related to game rules
        // --------------------------------------------
        // 1. 50-move rule
        // 2. Repetitions

        // 50-move rule
        if (virtualBoard.halfmovesClocked() >= 100)
            return 0;
        
        // Repetitions
        if (virtualBoard.irreversibleMoveDistance() >= 4) {
            std::uint16_t repetitions = virtualBoard.countRepetitions();

            if (repetitions == 3)   // A definite 3-fold
                return 0;
            if (repetitions == 2 && ssTop->ply > virtualBoard.irreversibleMoveDistance())    // A repetition inside search space
                return 0;
        }

        // Stage 2 - transposition table probe
        // -----------------------------------
        // 1. If score is exact (pv or terminal node), or if score holds in other types of nodes, perform a cut-off
        // 2. Otherwise try transposition table best / refutation move

        Move ttMove;
        const TranspositionTable::Entry* entry = tTable->probe(virtualBoard.hash(), virtualBoard.pieces());

        if (entry) {
            // Debug info
            if constexpr (SEARCH_MODE == TRACE && node == ROOT_NODE) {
                std::cout << "[TestEngine]: found existing entry in T.Table: [score: " << std::dec << relative_score(entry->score, &virtualBoard);
                std::cout << ", type: " << int(entry->typeOfNode) << ", best " << entry->bestMove << "]\n";
            }

            if (entry->typeOfNode == TERMINAL_NODE ||
                entry->depth >= depth && (entry->typeOfNode == PV_NODE || 
                                          entry->typeOfNode == CUT_NODE && entry->score >= beta ||
                                          entry->typeOfNode == ALL_NODE && entry->score <= alpha))
                return entry->score;

            ttMove = entry->bestMove;
            if (depth > 0 && ttMove != Move::null() && virtualBoard.fullLegalityTest(ttMove)) {
                // DEBUG
                if constexpr (SEARCH_MODE == TRACE && node == ROOT_NODE)
                    std::cout << ttMove << std::dec << " -> reduction: " << 1 << " | history: " << 
                                           history->score(&virtualBoard, ttMove) <<"\n";

                virtualBoard.makeMove(ttMove);
                Value score = -search<PV_NODE, true>(-beta, -alpha, depth - 1);
                virtualBoard.undoLastMove();
                ssTop--;

                if (score >= beta) {
                    tTable->set({
                        virtualBoard.hash(),
                        virtualBoard.pieces(),
                        depth,
                        score,  // score
                        ttMove,
                        CUT_NODE,
                        virtualBoard.halfmovesPlain()
                    }, rootAge);

                    // Killer heuristic update
                    if (ttMove.isQuiet() || SEE::evaluate(&virtualBoard, ttMove) < 0)
                        ssTop->saveKiller(ttMove);
                    
                    // History heuristic update
                    if (ttMove.isQuiet())
                        history->update(&virtualBoard, ttMove, HISTORY_FACTOR * depth * depth);

                    return score;
                }

                if (score > ssTop->score) {
                    ssTop->score = score;
                    ssTop->bestMove = ttMove;
                    if (score > alpha) {
                        alpha = score;
                        ssTop->node = PV_NODE;
                    }
                }

                // Update move counters
                if (!ttMove.isQuiet())                  ssTop->captureCounter++;
                else if (virtualBoard.isCheck(ttMove))  ssTop->checkCounter++;
                else                                    ssTop->defaultCounter++;
            }

            ssTop->eval = entry->score;
        }

        // Stage 2.5 - quiescence search
        // -----------------------------
        // 1. Go to quiescence if maximum depth reached
        // 2. Do not go further into search routine

        if (depth <= 0)
            return quiescence<ROOT_NODE>(alpha, beta, MAX_QUIESCENCE_DEPTH);
        
        // Stage 3 - static eval
        // ---------------------
        // 1. Statically evaluate current position
        // 2. Use obtained knowledge to improve further move ordering and apply various heuristics (Futility Pruning, etc.)
        
        ssTop->staticEval = evaluate();

        // Stage 4 - NMP (Null Move Pruning) heuristic
        // -------------------------------------------
        // 1. Check the conditions for NMP
        // 2. If conditions are met and cut-node is likely to happen in currect ply, perform search at lower depth
        // 3. If obtained score exceeds beta, perform a cut-off

        // Note: Since it is used before stealmate condition check, it might make some bugs in extremaly rare positions.
        //       Luckily it does not affect checkmate positions, since it cannot be used when side on move is in check.
        if constexpr (false && nmpAvailable) {
            // Null move pruning failes in zugzwang, so we want to avoid using it in late endgames, where zugzwang positions are most common.
            if (!virtualBoard.isInCheck() && virtualBoard.gameStage() > Evaluation::SINGLE_ROOK_VS_ROOK_ENDGAME) {
                int threats = evaluator.e_countThreatsAgainst_c(virtualBoard.movingSide());
                Value betterEval = ssTop->eval != NO_EVAL ? ssTop->eval : ssTop->staticEval;

                if (depth > 1 && threats == 0 && betterEval - NPM_ACTIVATION_THRESHOLD > beta) {
                    virtualBoard.makeNullMove();

                    Value score = -search<NON_PV_NODE, false>(-beta, -beta + 1, depth - 1);
                    ssTop--;

                    // If score already exceeds beta (with side on move playing one move less), then beta-cutoff is almost sure
                    if (score >= beta) {
                        virtualBoard.undoNullMove();
                        tTable->set({
                            virtualBoard.hash(),
                            virtualBoard.pieces(),
                            depth,
                            score,  // score
                            Move::null(),
                            CUT_NODE,
                            virtualBoard.halfmovesPlain()
                        }, rootAge);

                        return score;
                    }

                    virtualBoard.undoNullMove();
                }
            }
        }

        // Stage 5 - Initial move generation
        // ---------------------------------
        // - Generate moves according to current position, depth and strategy
        // - Sort moves by evaluation at initial depth (root node)

        MoveSelection::Selector moveSelector(&virtualBoard, &evaluator,
            virtualBoard.isInCheck() ? MoveGeneration::CHECK_EVASION : MoveGeneration::CAPTURE
        );

        // Update move counters
        // - If current phase is aggregative (check evasion), then we should count moves only with default counter
        if (moveSelector.phase() == MoveGeneration::CHECK_EVASION)
            ssTop->defaultCounter += ssTop->captureCounter + ssTop->checkCounter;

        // Stage 6 - checkmate & stealmate detection
        // -----------------------------------------
        // 1. Detect checkmate and stealmate which result in terminal node

        // Detect mate, stealmate and other types of draw
        if (!moveSelector.hasNext()) {
            Value score = virtualBoard.isInCheck() ? -MAX_EVAL : 0;      // Mate or stealmate
            tTable->set({
                virtualBoard.hash(),
                virtualBoard.pieces(),
                depth,
                score,
                Move::null(),
                TERMINAL_NODE,
                virtualBoard.halfmovesPlain()
            }, rootAge);

            // The score is exact, so we can finish searching the branch here
            return score;
        }

        if constexpr (SEARCH_MODE == TRACE && node == ROOT_NODE)
            std::cout << "Analyzing the following moves:\n";
        
        // Stage 7 - main search loop
        // --------------------------

        // Flow control variables
        EnhancedMove move;
        int nextKiller = 0;

        // LMR variables
        uint8_t* currentCounter = &ssTop->defaultCounter;
        uint8_t noMovesEstimate = std::ceil(float(moveSelector.size()) * 4.f / 5.f);
        float currentPhaseFactor = LMR_DEFAULT_FACTOR;

        // Quiets historical list (history heuristic)
        LightList<Move, MAX_NO_STORED_QUIETS> quiets;

        if (ttMove != Move::null()) {
            moveSelector.exclude(ttMove);

            if (ttMove.isQuiet())
                quiets.push_back(ttMove);
        }

        while (true) {

            // Stage 8 - move ordering strategies
            // ----------------------------------

            if (move == Move::null() && moveSelector.phase() == MoveGeneration::CAPTURE) {
                moveSelector.sort([this](const Move& move) -> int32_t {
                    return 1 * SEE::evaluate(&this->virtualBoard, move) +
                           64 * this->evaluator.e_isAvoidingThreats_c(move);
                });

                currentCounter = &ssTop->captureCounter;
                noMovesEstimate = std::ceil(float(moveSelector.size()) * 4.f / 5.f);
                currentPhaseFactor = LMR_CAPTURE_FACTOR;
            }

            else if (move == Move::null() && moveSelector.phase() == MoveGeneration::QUIET_CHECK) {
                moveSelector.sort([this, depth](const Move& move) -> int32_t {
                    return 1 * this->history->score(&this->virtualBoard, move) +
                           24 * (this->ssTop->ply + depth) * this->evaluator.e_isAvoidingThreats_c(move) +
                           64 * (this->ssTop->ply + depth) * this->evaluator.e_isCreatingThreats_c(move) +
                           128 * (this->ssTop->ply + depth) * this->evaluator.e_isSafe_h(move);
                    });

                currentCounter = &ssTop->checkCounter;
                noMovesEstimate = std::ceil(float(moveSelector.size()) * 4.f / 5.f);
                currentPhaseFactor = LMR_CHECK_FACTOR;
            }

            else if (move == Move::null() && moveSelector.phase() == MoveGeneration::QUIET) {
                moveSelector.sort([this, depth](const Move& move) -> int32_t {
                    return 1 * this->history->score(&this->virtualBoard, move) +
                           32 * (this->ssTop->ply + depth) * this->evaluator.e_isAvoidingThreats_c(move) +
                           32 * (this->ssTop->ply + depth) * this->evaluator.e_isCreatingThreats_c(move) +
                           128 * (this->ssTop->ply + depth) * this->evaluator.e_isSafe_h(move);
                });

                currentCounter = &ssTop->defaultCounter;
                noMovesEstimate = std::ceil(float(moveSelector.size()) * 4.f / 5.f);
                currentPhaseFactor = LMR_QUIET_FACTOR;
            }

            else if (move == Move::null() && moveSelector.phase() == MoveGeneration::CHECK_EVASION) {
                moveSelector.sort([this, depth](const Move& move) -> int32_t {
                    return 10000 * SEE::evaluate(&this->virtualBoard, move) +
                           1 * this->history->score(&this->virtualBoard, move) +
                           64 * (this->ssTop->ply + depth) * this->evaluator.e_isAvoidingThreats_c(move);
                });

                currentCounter = &ssTop->defaultCounter;
                noMovesEstimate = std::ceil(float(moveSelector.size()) * 4.f / 5.f);
                currentPhaseFactor = LMR_CHECK_EVASION_FACTOR;
            }

            move = moveSelector.next(MoveSelection::Selector::PARTIAL_CASCADE);

            if (move == Move::null() && moveSelector.phase() != MoveGeneration::NONE)
                continue;
            else if (move == Move::null())
                break;

            uint8_t* counter = currentCounter;

            // Stage 9 - killer move heuristic
            // --------------------------------

            bool useKiller = false;
            
            // We consider killer moves right after winning captures
            if (nextKiller < MAX_NO_KILLERS && SEE::evaluate(&virtualBoard, move) <= 0) {
                const Move& killer = ssTop->killers[nextKiller];

                // Check legality of the killer move (full check, since killer might not even be pseudolegal in current position)
                if (killer != Move::null() && killer != move && virtualBoard.fullLegalityTest(killer)) {
                    useKiller = true;

                    // Let's save generated move for later use
                    moveSelector.restoreLastMove();

                    // Switch to the killer move
                    move = killer;

                    // Exclude killer move from selection to avoid repeating the same move later
                    moveSelector.exclude(killer);

                    // Change move counters
                    if (moveSelector.phase() == MoveGeneration::CAPTURE || moveSelector.phase() == MoveGeneration::QUIET_CHECK) {
                        counter = !killer.isQuiet() ?            &ssTop->captureCounter :
                                  virtualBoard.isCheck(killer) ? &ssTop->checkCounter :
                                                                 &ssTop->defaultCounter;
                    }
                }

                nextKiller++;
            }

            // Stage 10 - futility pruning
            // ---------------------------
            // - Discard quiet moves with no perspective of raising alpha
            // - Since from depth 2 each side can make only 1 move, we can treat depth 2 the same as depth 1 with exception for zugzwang

            Value margin = depth == 1 ? FUTILITY_MARGIN_I : FUTILITY_MARGIN_II;

            if (depth <= 2 &&
                ssTop->staticEval + margin < alpha &&
                !virtualBoard.isInCheck() && move.isQuiet() && !virtualBoard.isCheck(move))
                continue;

            // Stage 11 -late move reduction
            // -----------------------------

            Depth reduction = 1;

            if (useLMR && depth > 2 && !useKiller) {
                float reductionIndex = float(*counter) * currentPhaseFactor / (LMR_UNIFIER / currentPhaseFactor + noMovesEstimate);

                reduction = 1 + std::min(1.f, lmr_function(reductionIndex)) * (float(depth) / 2 - 1);
            }
            
            // Make move and evaluate further
            virtualBoard.makeMove(move);
            Value score = -search<PV_NODE, true>(-beta, -alpha, depth - reduction);

            if (reduction > 1 && score > alpha) {
                score = -search<PV_NODE, true>(-beta, -alpha, depth - 1);
                ssTop--;
            }

            virtualBoard.undoLastMove();
            ssTop--;

            if constexpr (SEARCH_MODE == TRACE && node == ROOT_NODE)
                std::cout << move << std::dec << " -> reduction: " << int(reduction) << " | history: " << 
                                     history->score(&virtualBoard, move) <<"\n";
                //std::cout << move << std::dec << " -> [score: " << relative_score(score, &virtualBoard) << "]\n";

            // Beta cut-off: an enemy would never allow a line worse than beta to occur
            if (score >= beta) {
                tTable->set({
                    virtualBoard.hash(),
                    virtualBoard.pieces(),
                    depth,
                    score,  // score
                    move,
                    CUT_NODE,
                    virtualBoard.halfmovesPlain()
                }, rootAge);

                // Killer heuristic update
                if (move.isQuiet() || SEE::evaluate(&virtualBoard, move) < 0)
                    ssTop->saveKiller(move);
                
                // History heuristic update
                if (move.isQuiet()) {
                    // Give bonus for move that caused cut-off
                    history->update(&virtualBoard, move, HISTORY_FACTOR * depth * depth);

                    // Give penalty for all previously searched quiet moves
                    for (const Move& quiet : quiets)
                        history->update(&virtualBoard, quiet, -HISTORY_FACTOR * depth * depth * depth / (ssTop->ply + 1));
                }

                return score;
            }
            else if (move.isQuiet() && quiets.size() < MAX_NO_STORED_QUIETS)
                quiets.push_back(move);

            // PV node - a move that is better for moving side than previous best one
            if (score > ssTop->score) {
                ssTop->score = score;
                ssTop->bestMove = move;
                if (score > alpha) {
                    alpha = score;
                    ssTop->node = PV_NODE;
                }
            }

            // Update move counters
            (*counter)++;
        }

        // Save search results in transposition table
        tTable->set({
            virtualBoard.hash(),
            virtualBoard.pieces(),
            depth,
            ssTop->score,
            ssTop->bestMove,
            ssTop->node,
            virtualBoard.halfmovesPlain()
        }, rootAge);

        if (ssTop->bestMove != Move::null() && ssTop->bestMove.isQuiet()) {
            for (const Move& quiet : quiets)
                if (quiet != ssTop->bestMove) 
                    history->update(&virtualBoard, quiet, -HISTORY_FACTOR * depth * depth / int(quiets.size() - 1));
            history->update(&virtualBoard, ssTop->bestMove, HISTORY_FACTOR * depth * depth);
        }

        return ssTop->score;
    }


    // -----------------------------------
    // Crawler methods - quiescence search
    // -----------------------------------

    template <Node node>
    Value Crawler::quiescence(Value alpha, Value beta, Depth depth)
    {
        qs_nodes++;

        if constexpr (node != ROOT_NODE)
            pushStack();
        
        if constexpr (node != ROOT_NODE) {
            const TranspositionTable::Entry* entry = tTable->probe(virtualBoard.hash(), virtualBoard.pieces());
            if (entry && (entry->typeOfNode == PV_NODE || entry->typeOfNode == TERMINAL_NODE ||
                          entry->typeOfNode == CUT_NODE && entry->score >= beta ||
                          entry->typeOfNode == ALL_NODE && entry->score <= alpha))
                return entry->score;
        }

        MoveSelection::Selector moveSelector(&virtualBoard, &evaluator,
                                             virtualBoard.isInCheck() ? MoveGeneration::CHECK_EVASION : MoveGeneration::CAPTURE);

        // Detect mate, stealmate and other types of draw
        if (!moveSelector.hasNext()) {
            Value score = virtualBoard.isInCheck() ? -MAX_EVAL : 0;      // Mate or stealmate
            tTable->set({
                virtualBoard.hash(),
                virtualBoard.pieces(),
                depth,
                score,
                Move::null(),
                TERMINAL_NODE,
                virtualBoard.halfmovesPlain()
            }, rootAge);

            // The score is exact, so we can finish searching the branch here
            return score;
        }

        // Stand pat score - a static evaluation to help asses whether some moves are worth trying out or not
        ssTop->staticEval = evaluate();

        // Maximum queiescence depth reached
        if (depth == 0)
            return ssTop->staticEval;

        EnhancedMove move;

        // Check all "good" captures which could boost the score near the alpha (upper bound) region
        if (moveSelector.phase() >= MoveGeneration::CAPTURE) {
            // Experimental - sort moves by SEE at initial quiescence depth
            if constexpr (node == ROOT_NODE)
                moveSelector.sort([this](const Move& move) { return SEE::evaluate(&this->virtualBoard, move); }, EnhancementMode::PURE_SEE);
            else
                MoveSelection::improved_ordering(moveSelector);

            move = moveSelector.next(MoveSelection::Selector::STRICT);
            while (SEE::evaluate_save(&virtualBoard, move) > 0) {
                // Delta pruning
                if (ssTop->staticEval + move.see() + DELTA_MARGIN < alpha && moveSelector.phase() != MoveGeneration::CHECK_EVASION) {
                    if constexpr (node == ROOT_NODE)
                        break;
                    else {
                        move = moveSelector.next(MoveSelection::Selector::STRICT);
                        continue;
                    }
                }

                virtualBoard.makeMove(move);
                Value score = -quiescence<NON_PV_NODE>(-beta, -alpha, depth - 1);
                virtualBoard.undoLastMove();
                ssTop--;

                if (score >= beta)
                    return score;
                
                if (score > ssTop->score) {
                    ssTop->score = score;
                    if (score > alpha)
                        alpha = score;
                }
                
                move = moveSelector.next(MoveSelection::Selector::STRICT);
            }
        }

        int noThreats = evaluator.e_countThreatsAgainst_c(virtualBoard.movingSide());
        Bitboard threats = evaluator.e_threatsAgainst_c(virtualBoard.movingSide());

        // Consider moving threatened pieces or capture attacking pieces to stabilize
        if (ssTop->staticEval + EPSILON_MARGIN > alpha && (noThreats > 1 || virtualBoard.isInCheck())) {
            if constexpr (node != ROOT_NODE)
                MoveSelection::simple_ordering(moveSelector);

            if (move == Move::null())
                move = moveSelector.next(MoveSelection::Selector::FULL_CASCADE);
            while (move != Move::null()) {
                PieceType pType = type_of(virtualBoard.onSquare(move.to()));
                Bitboard attacks = pType == NULL_TYPE ? 0 :
                                   pType == PAWN ? Pieces::pawn_attacks(~virtualBoard.movingSide(), move.to()) :
                                                   Pieces::piece_attacks_d(pType, move.to(), virtualBoard.pieces());
                if (virtualBoard.isInCheck() ||
                    moveSelector.phase() != MoveGeneration::CAPTURE && threats & move.from() ||
                    moveSelector.phase() == MoveGeneration::CAPTURE &&
                    ssTop->staticEval + EPSILON_MARGIN + SEE::evaluate(&virtualBoard, move) > alpha &&
                    (threats & move.from() || attacks & threats)
                ) {
                    virtualBoard.makeMove(move);
                    Value score = -quiescence<NON_PV_NODE>(-beta, -alpha, depth - 1);
                    virtualBoard.undoLastMove();
                    ssTop--;

                    if (score >= beta)
                        return score;
                    
                    if (score > ssTop->score) {
                        ssTop->score = score;
                        if (score > alpha)
                            alpha = score;
                    }
                }

                move = moveSelector.next(MoveSelection::Selector::FULL_CASCADE);
            }

            return ssTop->score;
        }
        
        return std::max(ssTop->staticEval, ssTop->score);
    }

}