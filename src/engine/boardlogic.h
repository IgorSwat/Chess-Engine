#pragma once

#include "boardspace.h"


/*
    ---------- Board logic ----------

    Similarly to board space, this file works as an additional library that covers pure game rule aspects of chess board
    - Contains helper functions and tables for game rule aspects such as castling
*/

namespace Chessboard {

    // ---------------------
	// Board logic - general
	// ---------------------

    constexpr inline Square king_starting_position(Color side)
    {
        return side == WHITE ? SQ_E1 : SQ_E8;
    }


    // ----------------------
	// Board logic - castling
	// ----------------------

    // Castle has some fixed properties, like target files for king
    constexpr File OO_FILE = ::FILE_G;
    constexpr File OOO_FILE = ::FILE_C;

    // Calculate castling target square for king
    constexpr inline Square castle_target(Color side, Castle castle)
    {
        // Since side is either 0 (WHITE) or 1 (BLACK), the rank is mapped to either RANK_1 or RANK_8
        return make_square(Rank(::RANK_8 * uint32_t(side)), castle == KINGSIDE_CASTLE ? OO_FILE : OOO_FILE);
    }

    // Reverse function - castle type based on target square for the king
    constexpr inline Castle castle_type(Square king_to)
    {
        // Simple comparisions should be most efficient in this case
        return king_to == SQ_G1 || king_to == SQ_G8 ? KINGSIDE_CASTLE :
               king_to == SQ_C1 || king_to == SQ_C8 ? QUEENSIDE_CASTLE : NULL_CASTLE;
    }

    // Calculate castling path, which needs to be cleared of pieces to enable castling
    // - NOTE: does not include king's starting square
    inline Bitboard castle_path(Color side, Castle castle)
    {
        return side == WHITE ? (castle == KINGSIDE_CASTLE ? 0x0000000000000060 : 0x000000000000000e) :
                               (castle == KINGSIDE_CASTLE ? 0x6000000000000000 : 0x0e00000000000000);
    }

    // Lookup table - castling right loss after moving from square
    // - Defines which castle rights are being lost after entering or moving away from given square
    // - Since castling right loss is irreversible, only the first time we move away king or rook really matters
    // - From previously mentioned reason, we can always connect moving from square like A1 with moving white's rook
    // - Moving the king (moves related to E1 or E8 squares) results in loss of all castling rights for given side
    inline constexpr CastlingRights CastleLoss[SQUARE_RANGE] = {
        WHITE_OOO, NO_CASTLING, NO_CASTLING, NO_CASTLING, WHITE_BOTH, NO_CASTLING, NO_CASTLING, WHITE_OO,
        NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING,
        NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING,
        NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING,
        NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING,
        NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING,
        NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING, NO_CASTLING,
        BLACK_OOO, NO_CASTLING, NO_CASTLING, NO_CASTLING, BLACK_BOTH, NO_CASTLING, NO_CASTLING, BLACK_OO
    };

}