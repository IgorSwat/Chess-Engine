#pragma once

#include "eval.h"
#include "evalconfig.h"
#include <cmath>
#include <numbers>


// ---------------------------------
// Search parameters - search limits
// ---------------------------------

// A total maximum search depth is a sum of MAX_SEARCH_DEPTH and MAX_QUIESCENCE_DEPTH
constexpr int8_t MAX_SEARCH_DEPTH = 50;
constexpr int8_t MAX_QUIESCENCE_DEPTH = 10;
constexpr int8_t MAX_TOTAL_SEARCH_DEPTH = MAX_SEARCH_DEPTH + MAX_QUIESCENCE_DEPTH;


// ---------------------------------
// Search parameters - move ordering
// ---------------------------------

// History heuristic - score bounds
constexpr int HISTORY_MIN_SCORE = 0;
constexpr int HISTORY_MAX_SCORE = 1024;     // This is simultaneously a quantization factor for history formula

// History heuristic - eval normalization
// - Since history score values are relative to best move eval, there is a need normalize those results.
//   Otherwise, small shifts in small evaluations like +0.1 to +0.05 would produce drastic score changes
constexpr int HISTORY_EVAL_NORMALIZATION = 20;  // 20 cp

// History heuristic - importance factors
// - Two digit precision - all the numbers are divided by 100 during calculations
constexpr int HISTORY_PV_FACTOR = 32;
constexpr int HISTORY_ALL_FACTOR = 4;
constexpr int HISTORY_CUT_FACTOR = 16;

// History heuristic - number of analyzed moves (moves_tried list size)
constexpr int HISTORY_NO_MOVES = 64;

// Killer heuristic
constexpr int NO_KILLERS = 2;


// ----------------------------------------------
// Search parameters - futility pruning heuristic
// ----------------------------------------------

constexpr Evaluation::Eval FUTILITY_MARGIN_I = 200;
constexpr Evaluation::Eval FUTILITY_MARGIN_II = 500;


// ---------------------------------
// Search parameters - NMP heuristic
// ---------------------------------


// ---------------------------------
// Search parameters - LMR heuristic
// ---------------------------------

// Move index offset
// - Decides when to start applying LMR changes to depth reduction (typically at move number 2 + k when using log function)
constexpr int LMR_ACTIVATION_OFFSET = 2;

// Reduction factors
// - Higher factor = quicker reduction growth
constexpr double LMR_QUIET_FACTOR = 1.2;
constexpr double LMR_CHECK_FACTOR = 0.6;
constexpr double LMR_CAPTURE_FACTOR = 0.7;


// --------------------------------
// Queiescence parameters - pruning
// --------------------------------

// Delta pruning works similarly to futility pruning, but in queiscence search
constexpr Evaluation::Eval DELTA_MARGIN = 200;

// Quiescence stop parameter
constexpr Evaluation::Eval EPSILON_MARGIN = 50;