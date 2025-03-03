#pragma once

#include "board.h"

/*
    ---------- Movegen ----------

    A library focusing on generation of moves for given position
    - Specyfic generation modes allow to generate groups of moves that meet given property, like check evasions or captures
*/

namespace MoveGeneration {

    // --------------------------
    // Generation mode definition
    // --------------------------

    // Decides on which groups of moves should be generated
    enum Mode : uint32_t {
        NONE = 0,

        // Specyfic generation
        QUIET = 1,
        QUIET_CHECK,
        CAPTURE,
        CHECK_EVASION,      // This one should be classified as hybrid - it's simultaneously specyfic and collective generation

        // Collective generation
        PSEUDO_LEGAL,       // All the moves that would pass is_pseudolegal() check
        LEGAL,              // All the moves that are legal in given position

        MODE_RANGE = 7
    };


    // ----------------------------
    // Move generation - collective
    // ----------------------------

    // The below algorithms focuses on generating groups of moves, where only input is a chess position
    // - Moves are being saved on given Move list
    // - Covers most of use cases inside the engine

    // Generate moves according to given position and add them to movelist
    template <Mode mode, typename MoveT = Move>
	void generate_moves(const Board& board, Moves::List<MoveT>& movelist);


    // ----------------------------
    // Move generation - individual
    // ----------------------------

    // Individual generation focuses on creating a single move instance from given information
    // - For example, creates a pseudo-legal move based on given start and target square in the context of given position
    // - Used mostly for secondary purposes, like GUI implementation or some testing

    // Create a pseudolegal move by applying appropriate flags in the context of given position
    Move create_move(const Board& board, Square from, Square to);

}