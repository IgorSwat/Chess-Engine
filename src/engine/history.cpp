#include "history.h"
#include <algorithm>


namespace Search {

    // ---------------
    // History methods
    // ---------------

    void History::reset()
    {
        for (int i = 0; i < PIECE_RANGE; i++) {
            std::fill(scores[i], scores[i] + SQUARE_RANGE, 0);
            std::fill(trials[i], trials[i] + SQUARE_RANGE, 0);
        }
    }

    // History aging
    void History::flatten(int factor)
    {
        for (int i = 0; i < PIECE_RANGE; i++) {
            for (int j = 0; j < SQUARE_RANGE; j++) {
                scores[i][j] >>= factor;
                trials[i][j] >>= factor;
            }
        }
    }

}