#include <iostream>
#include "src/engine/pieces.h"
#include "src/engine/zobrist.h"
#include "test/test.h"

int main()
{
    // --------------------
    // Initialization stage
    // --------------------

    Board::initialize_board_space();
    Pieces::initialize_attack_tables();
    Zobrist::initialize_zobrist_numbers();
    // ....


    // -------------
    // Testing stage
    // -------------

    bool test_results = Testing::run_tests();

    if (!test_results)
        std::abort();


    // ---------------
    // Main processing
    // ---------------

    // ...

    return 0;
}