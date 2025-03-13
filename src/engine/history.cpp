#include "history.h"
#include <immintrin.h>


namespace Search {

    // --------------------------
    // History - global modifiers
    // --------------------------

    void History::flatten(unsigned factor)
    {
        // SIMD intrinsics for a little bit of speedup
        for (int i = 0; i < PIECE_RANGE; i++) {
            for (int j = 0; j < SQUARE_RANGE; j += 4) {
                // Update scores
                __m256i score_vals = _mm256_load_si256(reinterpret_cast<const __m256i*>(&m_scores[i][j]));
                __m256i score_shr = _mm256_srli_epi64(score_vals, factor);      // Right shift = division by 2^factor
                _mm256_store_si256(reinterpret_cast<__m256i*>(&m_scores[i][j]), score_shr);

                // Update counters
                __m128i counter_vals = _mm_load_si128(reinterpret_cast<const __m128i*>(&m_counters[i][j]));
                __m128i counter_shr = _mm_srli_epi32(counter_vals, factor);     // Right shift = division by 2^factor
                _mm_store_si128(reinterpret_cast<__m128i*>(&m_counters[i][j]), counter_shr);
            }
        }
    }

}