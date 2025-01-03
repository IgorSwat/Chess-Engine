#pragma once

#include "evaluationConfig.h"


// ---------------------------------------
// Search parameters - search testing mode
// ---------------------------------------

enum SearchMode {
    STANDARD = 0,
    TRACE,
    STATS
};

constexpr SearchMode SEARCH_MODE = STATS;


// ---------------------------------
// Search parameters - search limits
// ---------------------------------

// A total maximum search depth is a sum of MAX_SEARCH_DEPTH and MAX_QUIESCENCE_DEPTH
constexpr int MAX_SEARCH_DEPTH = 50;
constexpr int MAX_QUIESCENCE_DEPTH = 10;

constexpr Value MAX_EVAL = 1e4;
constexpr Value NO_EVAL = MAX_EVAL + 1;


// --------------------------------------------
// Search parameters - move ordering heuristics
// --------------------------------------------

constexpr int MAX_NO_KILLERS = 2;


// -------------------------------------
// Search parameters - null move pruning
// -------------------------------------

constexpr int NPM_MAX_USAGES = 0;               // Maximum number of available null move usages inside one branch
constexpr int NPM_CHECK_MIN_DEPTH = 0;          // A minimum depth at which null move needs to pass `eval > beta` check to be applied
constexpr int NPM_ACTIVATION_THRESHOLD = 20;    // TODO: remove it


// ----------------------------------------
// Search parameters - late move reductions
// ----------------------------------------

constexpr bool ALLOW_LMR = false;


// ------------------------------------
// Search parameters - futility pruning
// ------------------------------------

constexpr Value FUTILITY_MARGIN = 150;


// ----------------------------------------------
// Search parameters - delta pruning (quiescence)
// ----------------------------------------------

constexpr Value DELTA_MARGIN = 150;     // Determines the delta pruning scale in quiescence
constexpr Value EPSILON_MARGIN = 50;    // Specifies the behavior (considering equal exchanges and anti-threat, quiet moves) of quiescence