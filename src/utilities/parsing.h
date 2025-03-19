#pragma once

#include "../engine/board.h"


/*
    ---------- Parsing ----------

    Common parsing library
    - Parsing various text representation of moves, squares & more
*/

namespace Parsing {

    // ----------------------
    // Primitive type parsing
    // ----------------------

    Square parse_square(const std::string& notation);

    // ------------
    // Move parsing
    // ------------

    // Parses a move from string notation, like Rxa7, O-O, c4, in the context of given board
    Move parse_move(const Board& board, const std::string& notation);

}