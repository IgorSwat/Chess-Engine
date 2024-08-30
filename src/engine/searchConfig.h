#pragma once

#include "search.h"


// -----------------
// Search parameters
// -----------------

// Quiescence search
constexpr int MAX_QUIESCENCE_DEPTH = 10;
constexpr Value DELTA_MARGIN = 150;     // Determines the delta pruning scale in quiescence
constexpr Value EPSILON_MARGIN = 50;    // Specifies the behavior (considering equal exchanges and anti-threat, quiet moves) of quiescence