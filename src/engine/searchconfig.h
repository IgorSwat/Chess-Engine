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


// ------------------------------------
// Search parameters - killer heuristic
// ------------------------------------

constexpr unsigned NO_KILLERS = 2;


// -------------------------------------
// Search parameters - history heuristic
// -------------------------------------

constexpr int64_t MAX_HISTORY_SCORE = 1e5;
constexpr int32_t HISTORY_FACTOR = 5;


// ----------------------------------------------
// Search parameters - futility pruning heuristic
// ----------------------------------------------

constexpr Evaluation::Eval FUTILITY_MARGIN_I = 150;
constexpr Evaluation::Eval FUTILITY_MARGIN_II = 300;


// ---------------------------------
// Search parameters - NMP heuristic
// ---------------------------------


// ---------------------------------
// Search parameters - LMR heuristic
// ---------------------------------

// Those factors decide how quickly does reduction value grow
// - Higher factor = quicker reduction growth
constexpr float LMR_DEFAULT_FACTOR = 2.4f;
constexpr float LMR_CAPTURE_FACTOR = 2.0f;
constexpr float LMR_CHECK_FACTOR = 1.8f;
constexpr float LMR_CHECK_EVASION_FACTOR = 0.7f;
constexpr float LMR_QUIET_FACTOR = 3.0f;

// Unifier is used to balance reduction growth for phases with low amount of moves
constexpr float LMR_UNIFIER = 4.0f;

// Reduction function
// - A distribution of reduction values
// - Requires usage of floating point numbers
inline float lmr_function(float x) { return 1.55f / (1.f + std::pow(std::numbers::e, -4.2f * (x - 0.6f))) - 
                                            1.55f / (1.f + std::pow(std::numbers::e, 4.2f * 0.6f)); }


// --------------------------------
// Queiescence parameters - pruning
// --------------------------------

// Delta pruning works similarly to futility pruning, but in queiscence search
constexpr Evaluation::Eval DELTA_MARGIN = 150;

// Quiescence stop parameter
constexpr Evaluation::Eval EPSILON_MARGIN = 50;