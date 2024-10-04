#pragma once

#include "search.h"


// -----------------
// Search parameters
// -----------------

enum SearchMode {
    STANDARD = 0,
    TRACE,
    STATS
};

constexpr SearchMode SEARCH_MODE = STATS;

// Main search
// NPM
constexpr int NPM_MAX_USAGES = 0;           // Maximum number of available null move usages inside one branch
constexpr int NPM_CHECK_MIN_DEPTH = 0;      // A minimum depth at which null move needs to pass `eval > beta` check to be applied
constexpr int NPM_ACTIVATION_THRESHOLD = 20;

// LMR
constexpr bool ALLOW_LMR = false;

// Quiescence search
constexpr int MAX_QUIESCENCE_DEPTH = 10;
constexpr Value DELTA_MARGIN = 150;     // Determines the delta pruning scale in quiescence
constexpr Value EPSILON_MARGIN = 50;    // Specifies the behavior (considering equal exchanges and anti-threat, quiet moves) of quiescence