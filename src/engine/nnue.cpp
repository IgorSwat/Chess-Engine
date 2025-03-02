#include "nnue.h"
#include <exception>
#include <fstream>


namespace Evaluation {

    // -------------------------
    // NNUE - loading parameters
    // -------------------------

    void NNUE::load(const std::string& filepath)
    {
        // Open binary input file
        std::ifstream file(filepath, std::ios::binary);

        if (!file.is_open())
            throw std::invalid_argument("ERROR: Cannot open file " + filepath);

        // Read network parameters
        // - Very important to read in correct order (weights before biases, full layer before another)
        file.read(reinterpret_cast<char*>(m_accumulator_weights), sizeof(m_accumulator_weights));
        file.read(reinterpret_cast<char*>(m_accumulator_biases), sizeof(m_accumulator_biases));

        // Now for every output bucket
        // - weights nr 1 -> bias nr 1 -> weights nr 2 -> bias nr 2 -> ...
        for (int i = 0; i < OUTPUT_BUCKETS; i++) {
            file.read(reinterpret_cast<char*>(m_output_weights[i]), sizeof(m_output_weights[i]));
            file.read(reinterpret_cast<char*>(&m_output_bias[i]), sizeof(m_output_bias[i]));
        }

        // Close file stream
        file.close();
    }


    // -------------------------------
    // NNUE - network updates - static
    // -------------------------------

    void NNUE::set(const Board::Board& board)
    {
        // Reset state pointers
        m_curr_ply = 0;
        m_last_ready_ply = 0;

        // Iterate over all accumulator's neurons
        for (int n_id = 0; n_id < ACCUMULATOR_SIZE; n_id++) {
            // Reset accumulator value for both accumulators
            // - We can already set value to the corresponding bias to save two additional operations
            m_acc_white[m_curr_ply].values[n_id] = m_accumulator_biases[n_id];
            m_acc_black[m_curr_ply].values[n_id] = m_accumulator_biases[n_id];

            // Iterate over all pieces
            // - This approach is a little bit faster than iterating over all possible squares
            Bitboard pieces = board.pieces();

            while (pieces) {
                Square sq = Bitboards::pop_lsb(pieces);
                Color side = color_of(board.on(sq));
                PieceType ptype = type_of(board.on(sq));

                Index index = {side, ptype, sq};

                // Update both accumulators
                // - NOTE: input values are 0/1, which means dot product simplifies into sum of corresponding weights
                // - NOTE: it's important to match accumulator with correct perspective (acc_white = WHITE perspective, acc_black = BLACK perspective)
                m_acc_white[m_curr_ply].values[n_id] += m_accumulator_weights[index(WHITE)][n_id];
                m_acc_black[m_curr_ply].values[n_id] += m_accumulator_weights[index(BLACK)][n_id];
            }
        }

        // Now each of accumulator's values are correctly calculated and network is ready to perform quick forward pass (and obtain eval score)
    }


    // --------------------------------
    // NNUE - network updates - dynamic
    // --------------------------------

    void NNUE::update(const Board::Board& board, const Move& move)
    {
        // Step 1 - get rid of all entries in updates stack
        updates[m_curr_ply].clear();

        // Step 2 - calculate and save every change that a move introduces to piece placement
        // - Additions should always be put before subtractions
        // - NOTE: two updates always corresponds to add_sub, three updates always corresponds to add_sub_sub, and similarly with four updates
        Color side = board.side_to_move();
        if (move.is_castle()) {
            Square king_from = move.from();
            Square king_to = move.to();
            Square rook_from = make_square(rank_of(king_to), file_of(king_to) == FILE_G ? FILE_H : FILE_A);
            Square rook_to = make_square(rank_of(king_to), file_of(king_to) == FILE_G ? FILE_F : FILE_D);

            updates[m_curr_ply].push_back({side, KING, king_to});
            updates[m_curr_ply].push_back({side, ROOK, rook_to});
            updates[m_curr_ply].push_back({side, KING, king_from});
            updates[m_curr_ply].push_back({side, ROOK, rook_from});
        }
        else if (move.is_enpassant()) {
            updates[m_curr_ply].push_back({side, PAWN, move.to()});
            updates[m_curr_ply].push_back({side, PAWN, move.from()});
            updates[m_curr_ply].push_back({~side, PAWN, board.enpassant_square()});
        }
        else {
            PieceType old_type = type_of(board.on(move.from()));
            PieceType new_type = move.is_promotion() ? move.promotion_type() : old_type;

            updates[m_curr_ply].push_back({side, new_type, move.to()});
            updates[m_curr_ply].push_back({side, old_type, move.from()});

            if (move.is_capture())
                updates[m_curr_ply].push_back({~side, type_of(board.on(move.to())), move.to()});
        }

        // Stgep 3 - increment state pointer
        m_curr_ply++;
    }

    // Helper function - making updates and prepering accumulators for a forward pass
    // - This function is called only when forward pass needs to be made
    void NNUE::make_updates()
    {
        // We want to incrementally update each accumulator up until the one pointed by ply pointer
        while (m_last_ready_ply < m_curr_ply) {
            // Select update function by checking size of the corresponding updates list
            // - NOTE: two updates always corresponds to add_sub, three updates always corresponds to add_sub_sub, and similarly with four updates
            if (updates[m_last_ready_ply].size() == 2) {
                m_acc_white[m_last_ready_ply + 1].add_sub(&m_acc_white[m_last_ready_ply], m_accumulator_weights,
                                                          updates[m_last_ready_ply][0](WHITE), updates[m_last_ready_ply][1](WHITE));
                m_acc_black[m_last_ready_ply + 1].add_sub(&m_acc_black[m_last_ready_ply], m_accumulator_weights,
                                                          updates[m_last_ready_ply][0](BLACK), updates[m_last_ready_ply][1](BLACK));
            }
            else if (updates[m_last_ready_ply].size() == 3) {
                m_acc_white[m_last_ready_ply + 1].add_sub_sub(&m_acc_white[m_last_ready_ply], m_accumulator_weights,
                                                              updates[m_last_ready_ply][0](WHITE), updates[m_last_ready_ply][1](WHITE),
                                                              updates[m_last_ready_ply][2](WHITE));
                m_acc_black[m_last_ready_ply + 1].add_sub_sub(&m_acc_black[m_last_ready_ply], m_accumulator_weights,
                                                              updates[m_last_ready_ply][0](BLACK), updates[m_last_ready_ply][1](BLACK),
                                                              updates[m_last_ready_ply][2](BLACK));
            }
            else {
                m_acc_white[m_last_ready_ply + 1].add_add_sub_sub(&m_acc_white[m_last_ready_ply], m_accumulator_weights,
                                                                  updates[m_last_ready_ply][0](WHITE), updates[m_last_ready_ply][1](WHITE),
                                                                  updates[m_last_ready_ply][2](WHITE), updates[m_last_ready_ply][3](WHITE));
                m_acc_black[m_last_ready_ply + 1].add_add_sub_sub(&m_acc_black[m_last_ready_ply], m_accumulator_weights,
                                                                  updates[m_last_ready_ply][0](BLACK), updates[m_last_ready_ply][1](BLACK),
                                                                  updates[m_last_ready_ply][2](BLACK), updates[m_last_ready_ply][3](BLACK));
            }

            // Mark processed ply as ready by incrementing the pointer
            m_last_ready_ply++;
        }
    }

    // Accumulator update functions - for quiet moves
    // - Uses AVX and AVX2 intrinsics to vectorize calculations
    void NNUE::Accumulator::add_sub(NNUE::Accumulator* prev, int16_t weights[][ACCUMULATOR_SIZE], 
                                    uint32_t add_idx, uint32_t sub_idx)
    {
        // Since 256-bit chunk of data contains 256 / 16 = 16 16-bit integers, we can increase loop step to 16
        for (int i = 0; i < ACCUMULATOR_SIZE; i += 16) {
            // Load chunks of data
            __m256i prev_vals = _mm256_load_si256(reinterpret_cast<const __m256i*>(&prev->values[i]));
            __m256i add_vals = _mm256_load_si256(reinterpret_cast<const __m256i*>(&weights[add_idx][i]));
            __m256i sub_vals = _mm256_load_si256(reinterpret_cast<const __m256i*>(&weights[sub_idx][i]));

            // Perform arithmetic operations
            // - in this case one addition and one subtraction, since we are in add_sub function
            __m256i result = _mm256_add_epi16(prev_vals, add_vals);
            result = _mm256_sub_epi16(result, sub_vals);

            // Save results
            _mm256_store_si256(reinterpret_cast<__m256i*>(&values[i]), result);
        }
    }

    // Accumulator update functions - for captures
    void NNUE::Accumulator::add_sub_sub(NNUE::Accumulator* prev, int16_t weights[][ACCUMULATOR_SIZE],
                                        uint32_t add_idx, uint32_t sub_idx_1, uint32_t sub_idx_2)
    {
        // Since 256-bit chunk of data contains 256 / 16 = 16 16-bit integers, we can increase loop step to 16
        for (int i = 0; i < ACCUMULATOR_SIZE; i += 16) {
            // Load chunks of data
            __m256i prev_vals = _mm256_load_si256(reinterpret_cast<const __m256i*>(&prev->values[i]));
            __m256i add_vals = _mm256_load_si256(reinterpret_cast<const __m256i*>(&weights[add_idx][i]));
            __m256i sub1_vals = _mm256_load_si256(reinterpret_cast<const __m256i*>(&weights[sub_idx_1][i]));
            __m256i sub2_vals = _mm256_load_si256(reinterpret_cast<const __m256i*>(&weights[sub_idx_2][i]));

            // Perform arithmetic operations
            // - in this case one addition and two subtractions, since we are in add_sub_sub function
            __m256i result = _mm256_add_epi16(prev_vals, add_vals);
            result = _mm256_sub_epi16(result, sub1_vals);
            result = _mm256_sub_epi16(result, sub2_vals);

            // Save results
            _mm256_store_si256(reinterpret_cast<__m256i*>(&values[i]), result);
        }
    }

    // Accumulator update functions - for castles
    void NNUE::Accumulator::add_add_sub_sub(NNUE::Accumulator* prev, int16_t weights[][ACCUMULATOR_SIZE],
                                            uint32_t add_idx_1, uint32_t add_idx_2, uint32_t sub_idx_1, uint32_t sub_idx_2)
    {
        // Since 256-bit chunk of data contains 256 / 16 = 16 16-bit integers, we can increase loop step to 16
        for (int i = 0; i < ACCUMULATOR_SIZE; i += 16) {
            // Load chunks of data
            __m256i prev_vals = _mm256_load_si256(reinterpret_cast<const __m256i*>(&prev->values[i]));
            __m256i add1_vals = _mm256_load_si256(reinterpret_cast<const __m256i*>(&weights[add_idx_1][i]));
            __m256i add2_vals = _mm256_load_si256(reinterpret_cast<const __m256i*>(&weights[add_idx_2][i]));
            __m256i sub1_vals = _mm256_load_si256(reinterpret_cast<const __m256i*>(&weights[sub_idx_1][i]));
            __m256i sub2_vals = _mm256_load_si256(reinterpret_cast<const __m256i*>(&weights[sub_idx_2][i]));

            // Perform arithmetic operations
            // - in this case two addition and two subtractions, since we are in add_add_sub_sub function
            __m256i result = _mm256_add_epi16(prev_vals, add1_vals);
            result = _mm256_add_epi16(result, add2_vals);
            result = _mm256_sub_epi16(result, sub1_vals);
            result = _mm256_sub_epi16(result, sub2_vals);

            // Save results
            _mm256_store_si256(reinterpret_cast<__m256i*>(&values[i]), result);
        }
    }


    // -------------------
    // NNUE - forward pass
    // -------------------

    int32_t NNUE::forward(const Board::Board& board)
    {
        __m256i v_eval = _mm256_setzero_si256();

        // Step 1 - make sure accumulators are properly updated and network is ready to calculate outputs
        make_updates();

        // Step 2 - order accumulators based on current side to move
        Accumulator* stm_acc = board.side_to_move() == WHITE ? &m_acc_white[m_curr_ply] : &m_acc_black[m_curr_ply];
        Accumulator* nstm_acc = board.side_to_move() == WHITE ? &m_acc_black[m_curr_ply] : &m_acc_white[m_curr_ply];

        // Step 3 - select appropriate output bucket based on number of non-king pieces on the board
        constexpr uint32_t divisor = (32 + OUTPUT_BUCKETS - 1) / OUTPUT_BUCKETS;
        uint32_t no_pieces = Bitboards::popcount(board.pieces()) - 2;
        uint32_t bucket_id = no_pieces / divisor;

        int16_t* output_weights = m_output_weights[bucket_id];
        int16_t output_bias = m_output_bias[bucket_id];

        // Step 4 - calculate dor product for output layer
        // - Vectorized calculations, 8 values at once
        for (int i = 0; i < ACCUMULATOR_SIZE; i += 8) {
            __m128i stm_vals = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&stm_acc->values[i]));
            __m128i nstm_vals = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&nstm_acc->values[i]));

            // Side to move related accumulator always goes first
            __m128i stm_weights = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&output_weights[i]));
            __m128i nstm_weights = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&output_weights[ACCUMULATOR_SIZE + i]));

            __m256i stm_activations = activation(stm_vals);
            __m256i nstm_activations = activation(nstm_vals);

            // Convert weights to 32-bit integers
            __m256i stm_weights_cvt = _mm256_cvtepi16_epi32(stm_weights);
            __m256i nstm_weights_cvt = _mm256_cvtepi16_epi32(nstm_weights);

            __m256i result1 = _mm256_mullo_epi32(stm_activations, stm_weights_cvt);
            __m256i result2 = _mm256_mullo_epi32(nstm_activations, nstm_weights_cvt);

            // Concatenate results and add to eval
            v_eval = _mm256_add_epi32(v_eval, result1);
            v_eval = _mm256_add_epi32(v_eval, result2);
        }

        // Step 5 - restore scalar evaluation from eval vector
        // - Since eval vector contains of 8 evaluation score parts, we must concatenate them back into one integer value
        int32_t tmp[8];
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(tmp), v_eval);

        int32_t eval = 0;
        for (int i = 0; i < 8; i++)
            eval += tmp[i];

        // Don't forget about bias
        eval += output_bias;

        // Step 6 - dequantization
        // - Because of quantization, eval is initially scalled by (QA * QB)
        // - WARNING: Be careful for implicit casts! (eval /= (QA * QB) produces critical errors when eval < 0 due to implicit casts)
        eval /= static_cast<int32_t>(QA * QB);

        return eval;
    }

}