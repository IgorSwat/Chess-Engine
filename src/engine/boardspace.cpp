#include "boardspace.h"
#include <array>


namespace Board {

    // -------------
	// Lookup tables
	// -------------

    Bitboard Paths[SQUARE_RANGE][SQUARE_RANGE] = { 0 };
    Bitboard Lines[SQUARE_RANGE][SQUARE_RANGE] = { 0 };
	Bitboard Boxes[SQUARE_RANGE][SQUARE_RANGE] = { 0 };
    Bitboard Vspans[SQUARE_RANGE][2] = { 0 };
    Bitboard Hspans[SQUARE_RANGE][2] = { 0 };

    // --------------
	// Initialization
	// --------------

    // Helper function nr 1
    // - This function is an equivalent to iterating over all directions and trying fill algorithm to detect paths
    template <Direction dir, Direction... rest>
    void calculate_lines(Square sq1, Square sq2)
    {
        if (Bitboards::fill<dir>(square_to_bb(sq1)) & sq2) {
            Paths[sq1][sq2] = Bitboards::fill<dir>(square_to_bb(sq1)) & Bitboards::fill<~dir>(square_to_bb(sq2));
            Lines[sq1][sq2] = Bitboards::fill<dir>(square_to_bb(sq1)) | Bitboards::fill<~dir>(square_to_bb(sq2));

            return;
        }
            
        if constexpr (sizeof...(rest) > 0)
            calculate_lines<rest...>(sq1, sq2);
    }

    // Helper function nr 2
    void calculate_boxes(Square sq1, Square sq2)
    {
        // Here from each square we create "quarters" which are pointed out towards another square
        // Fill algorithm applied in two steps fits perfectly for this task
        Bitboard fill1 = file_of(sq2) >= file_of(sq1) ? Bitboards::fill<EAST>(square_to_bb(sq1)) : 
                                                        Bitboards::fill<WEST>(square_to_bb(sq1));
		fill1 = rank_of(sq2) >= rank_of(sq1) ? Bitboards::fill<NORTH>(fill1) : 
                                               Bitboards::fill<SOUTH>(fill1);

		Bitboard fill2 = file_of(sq1) > file_of(sq2) ? Bitboards::fill<EAST>(square_to_bb(sq2)) :
                                                       Bitboards::fill<WEST>(square_to_bb(sq2));
		fill2 = rank_of(sq1) > rank_of(sq2) ? Bitboards::fill<NORTH>(fill2) :
                                              Bitboards::fill<SOUTH>(fill2);

        // Combine quartes to obtain common area - a box
        Boxes[sq1][sq2] = fill1 & fill2;
    }

    // Main iniitializer
    void initialize_board_space()
    {
        // Iterate over every possible square
        for (int sq1 = 0; sq1 < SQUARE_RANGE; sq1++) {
            // 1. Calculate properties that require only one square as input information

            // Vertical spans - 0 for south, 1 for north
            Bitboard rank_surrounding = adjacent_rank_squares(Square(sq1)) | Square(sq1);
            Vspans[sq1][0] = Bitboards::fill<SOUTH>(rank_surrounding) ^ rank_surrounding;
            Vspans[sq1][1] = Bitboards::fill<NORTH>(rank_surrounding) ^ rank_surrounding;

            // Horizontal spans - 0 for west, 1 for east
            Bitboard file_surrounding = adjacent_file_squares(Square(sq1)) | Square(sq1);
            Hspans[sq1][0] = Bitboards::fill<WEST>(file_surrounding) ^ file_surrounding;
            Hspans[sq1][1] = Bitboards::fill<EAST>(file_surrounding) ^ file_surrounding;

            // 2. Iterate again over every square and calculate pair properties
            for (int sq2 = 0; sq2 < SQUARE_RANGE; sq2++) {
                // Special case - sq1 = sq2
                if (sq1 == sq2) {
                    Paths[sq1][sq2] = square_to_bb(Square(sq1));
                    Boxes[sq1][sq2] = square_to_bb(Square(sq1));
                    // We do not calculate Lines for sq1 = sq2, since it would be ambiguous
                }
                else {
                    // Paths and lines
                    calculate_lines<NORTH, NORTH_EAST, EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, WEST, NORTH_WEST>(Square(sq1), Square(sq2));

                    // Boxes
                    calculate_boxes(Square(sq1), Square(sq2));
                }
            }
        }
    }

}