#pragma once

#include "utilities.h"
#include "../logic/boardConfig.h"


// ----------------------
// Common parsing library
// ----------------------

namespace Utilities::Parsing {

    // ----------------------
    // Primitive type parsing
    // ----------------------

    Square parse_square(const std::string& notation);

    // ------------
    // Move parsing
    // ------------

    // Parses a move from string notation, like Rxa7, O-O, c4, in the context of given board
    Move parse_move(const BoardConfig& board, const std::string& notation);

}