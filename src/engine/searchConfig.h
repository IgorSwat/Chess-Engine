#pragma once

#include "evaluationConfig.h"
#include "interpolation.h"


// ---------------------------------------
// Search parameters - search testing mode
// ---------------------------------------

enum SearchMode {
    STANDARD = 0,
    TRACE,
    STATS
};

constexpr SearchMode SEARCH_MODE = STANDARD;


// ---------------------------------
// Search parameters - search limits
// ---------------------------------

// A total maximum search depth is a sum of MAX_SEARCH_DEPTH and MAX_QUIESCENCE_DEPTH
constexpr int MAX_SEARCH_DEPTH = 50;
constexpr int MAX_QUIESCENCE_DEPTH = 10;

constexpr Value MAX_EVAL = 1e4;
constexpr Value NO_EVAL = MAX_EVAL + 1;


// ---------------------------------
// Search parameters - move ordering
// ---------------------------------

// Killer heuristic
constexpr int MAX_NO_KILLERS = 2;

// History heuristic
constexpr int MAX_HISTORY = 1e7;
constexpr int HISTORY_FACTOR = 16;


// ----------------------------------------
// Search parameters - late move reductions
// ----------------------------------------

constexpr bool ALLOW_LMR = true;

const Interpolation::NormalizedSigmoid lmr_function = Interpolation::NormalizedSigmoid(1.55f, 4.2f, 0.6f);

constexpr float LMR_DEFAULT_FACTOR = 2.4f;
constexpr float LMR_CAPTURE_FACTOR = 2.0f;
constexpr float LMR_CHECK_FACTOR = 1.8f;
constexpr float LMR_CHECK_EVASION_FACTOR = 0.7f;
constexpr float LMR_QUIET_FACTOR = 3.0f;

constexpr float LMR_UNIFIER = 4.0f;


// ------------------------------------
// Search parameters - futility pruning
// ------------------------------------

constexpr Value FUTILITY_MARGIN_I = 150;    // For depth 1
constexpr Value FUTILITY_MARGIN_II = 350;   // For depth 2


// ----------------------------------------------
// Search parameters - delta pruning (quiescence)
// ----------------------------------------------

constexpr Value DELTA_MARGIN = 150;     // Determines the delta pruning scale in quiescence
constexpr Value EPSILON_MARGIN = 50;    // Specifies the behavior (considering equal exchanges and anti-threat, quiet moves) of quiescence