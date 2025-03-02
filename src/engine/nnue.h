#pragma once

#include "board.h"
#include "../utilities/sarray.h"
#include <immintrin.h>


/*
    ---------- NNUE ----------

    Contains in-build representation of NNUE (Efficiently Updatable Neural Network)
    - The point of using neural network is to replace hand crafted evaluation with more reliable, automatic evaluation source
    - Move by move dynamic updates allow for relatively quick computation of forward pass despite over 1 milion network parameters
*/

namespace Evaluation {

    // -----------------
    // NNUE - parameters
    // -----------------

    // These parameters depend on used architecture and should be matched carefully with loaded network
    constexpr uint32_t INPUT_SIZE = 768;
    constexpr uint32_t ACCUMULATOR_SIZE = 1024;
    constexpr uint32_t OUTPUT_BUCKETS = 8;
    constexpr uint32_t OUTPUT_SIZE = 1;

    // Quantization factors
    constexpr uint32_t QA = 100;
    constexpr uint32_t QB = 100;

    // Other parameters
    constexpr uint32_t MAX_PLY = 50 + 1;


    // ---------------------
    // NNUE - input indexing
    // ---------------------

    // An index is composed of piece color, piece type and occupied square
    // - This gives us 2 * 6 * 64 = 768 possible combinations, and thus this is the input layer size
    // - The point of Index structure is to provide abstraction for input index while still grouping index components together
    struct Index
    {
    public:
        Color side;
        PieceType ptype;
        Square square;

        // Functor methods
        uint32_t operator()() const { return index(side, ptype, square); }
        uint32_t operator()(Color perspective) const { return perspective == BLACK ? index(~side, ptype, flip_rank(square)) : 
                                                                                     index(side, ptype, square); }
    private:
        static uint32_t index(Color s, PieceType pt, Square sq) { return s * 384 + (pt - 1) * 64 + sq; }
    };


    // --------------------------
    // NNUE - activation function
    // --------------------------

    // Activation function range
    // - We additionally multiply range by 6 because of using ReLU6 in initial network architecture during training
    constexpr int16_t ACTIVATION_RANGE = QA * 6;

    // Activation function - CReLU
    // - Vectorized, SIMD implementation
    // - Automatic conversion to 32-bit integer
    // - Takes 128 bits ( 8 x 16-bit integer) values as input
    inline __m256i activation(const __m128i& values)
    {
        __m128i max_vals = _mm_set1_epi16(ACTIVATION_RANGE);
        __m128i min_vals = _mm_setzero_si128();

        // Alternative implementation of std::clamp - using min + max SIMD intrinsics
        __m128i clamped_vals = _mm_min_epi16(values, max_vals);
        clamped_vals = _mm_max_epi16(clamped_vals, min_vals);

        // Convert to 32-bit integers
        return _mm256_cvtepi16_epi32(clamped_vals);
    }

    
    // ----
    // NNUE
    // ----

    // Represents an evaluation network
    class NNUE
    {
    public:
        NNUE() = default;

        // Loading network parameters
        void load(const std::string& filepath);

        // Network updates
        // - Static update (set): recalculates all accumulator values from scratch
        // - Dynamic update: updates existing accumulator values in dynamic mannor by adding or subtracting new piece-square value
        // - NOTE: at least one set() needs to be called at the start of network usage
        // - NOTE: update() does not really update accumulators - it only ensures, that position will be updated in the future (before evaluation)
        // - WARNING: update() assumes, that move has not been made yet (must be called before board.make_move())
        void set(const Board::Board& board);
        void update(const Board::Board& board, const Move& move);
        void undo_state() { m_curr_ply = std::max(0, m_curr_ply - 1); m_last_ready_ply = std::min(m_last_ready_ply, m_curr_ply); }

        // Forward pass
        int32_t forward(const Board::Board& board);

    private:
        // Helper functions - lazy update handlers
        void make_updates();

        // NNUE components - weights and biases
        // - Using 16-bit integers allows for better optimization of dynamic update calculation
        // - Alignment to 32 bits for AVX instructions effectivness
        // - NOTE: m_accumulator_weights has a bit of a counter-intuitive shape, but INPUT_SIZE must come first for intrinsics to work properly
        alignas(32) int16_t m_accumulator_weights[INPUT_SIZE][ACCUMULATOR_SIZE];
        alignas(32) int16_t m_accumulator_biases[ACCUMULATOR_SIZE];
        alignas(32) int16_t m_output_weights[OUTPUT_BUCKETS][2 * ACCUMULATOR_SIZE];
        alignas(32) int16_t m_output_bias[OUTPUT_BUCKETS];

        // NNUE components - accumulators
        // - Accumulator is a network layer wchich "accumulates" input values, storing them and allowing for dynamic update
        // - There are exactly two accumulators: one for white side, and one for black side
        // - Depending on who is on move we either treat acc_white or acc_black as side to move accumulator
        struct alignas(32) Accumulator
        {
            int16_t values[ACCUMULATOR_SIZE];

            // Those functions look quite ugly, but merging smaller ones into bigger ones allows for further optimization (fused updates)
            void add_sub(Accumulator* prev, int16_t weights[][ACCUMULATOR_SIZE], 
                         uint32_t add_idx, uint32_t sub_idx);
            void add_sub_sub(Accumulator* prev, int16_t weights[][ACCUMULATOR_SIZE],
                             uint32_t add_idx, uint32_t sub_idx_1, uint32_t sub_idx_2);
            void add_add_sub_sub(Accumulator* prev, int16_t weights[][ACCUMULATOR_SIZE],
                                 uint32_t add_idx_1, uint32_t add_idx_2, uint32_t sub_idx_1, uint32_t sub_idx_2);
        };

        // Accumulators are indexed by ply index
        // - This allows to optimize network since unmake move now requires just decrementing the ply pointer
        alignas(32) Accumulator m_acc_white[MAX_PLY];
        alignas(32) Accumulator m_acc_black[MAX_PLY];
        
        // NNUE components - update stack
        // - To consider NNUE as ready at ply P, all changes from updates[0] up to updates[P] (excluding updates[P]) must be applied
        // - This basically implements lazy updates, where changes are applied only when evaluation needs to be called, instead of after every move
        StableArray<Index, 4> updates[MAX_PLY];

        // State pointers
        int m_curr_ply = 0;        // Points to the top of accumulator and update stack
        int m_last_ready_ply = 0;  // Points to the last ply at which accumulators are properly updated
    };

}