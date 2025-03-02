#include "test.h"
#include "../src/engine/movegen.h"
#include <algorithm>


namespace Testing {

    // ------------------
    // Helper definitions
    // ------------------

    // A structure that aggregates all move counters
    struct Counters
    {
        uint32_t all_moves = 0;
        uint32_t captures = 0;
        uint32_t enpassants = 0;
        uint32_t castles = 0;
        uint32_t promotions = 0;

        void operator+=(const Counters& other)
        {
            all_moves += other.all_moves;
            captures += other.captures;
            enpassants += other.enpassants;
            castles += other.castles;
            promotions += other.promotions;
        }
    };


    // ----------------------
    // Move counting function
    // ----------------------

    // This is a recursive function, which goes through every node inside the search tree
    // - The goal is to count number of every possible legal moves that normally would be searched by mini-max algorithm
    // - Comparing with some of the known test positions and values allows to check the correctness of both Board and move generation logic
    Counters count_moves(Board::Board& board, uint32_t depth)
    {
        // All the counters are initially zeros
        Counters counters;

        // Generate all legal moves
        Moves::List movelist;
        MoveGeneration::generate_moves<MoveGeneration::LEGAL>(board, movelist);

        // Recursion's break condition
        if (depth == 1) {
            // Count each moves of each type
            counters.all_moves = uint32_t(movelist.size());
            counters.captures = std::count_if(movelist.begin(), movelist.end(), 
                                                    [](const Move& move) {return move.is_capture(); });
			counters.enpassants = std::count_if(movelist.begin(), movelist.end(), 
                                                    [](const Move& move) {return move.is_enpassant(); });
			counters.castles = std::count_if(movelist.begin(), movelist.end(), 
                                                    [](const Move& move) {return move.is_castle(); });
			counters.promotions = std::count_if(movelist.begin(), movelist.end(), 
                                                    [](const Move& move) {return move.is_promotion(); });
        }
        else {
            // Check all the branches
            for (const Move& move : movelist) {
                board.make_move(move);
                Counters subresult = count_moves(board, depth - 1);
                board.undo_move();

                counters += subresult;
            }
        }

        return counters;
    }


    // ---------------------
    // Move generation tests
    // ---------------------

    // A universal testing function that runs move counting for given position and compares to expected result
    // - Common part of all following tests
    bool test_generation(const std::string& fen, uint32_t depth, const Counters& expected)
    {
        Board::Board board;
        board.load_position(fen);

        Counters result = count_moves(board, depth);

        ASSERT_EQUALS(expected.all_moves, result.all_moves);
        ASSERT_EQUALS(expected.captures, result.captures);
        ASSERT_EQUALS(expected.enpassants, result.enpassants);
        ASSERT_EQUALS(expected.castles, result.castles);
        ASSERT_EQUALS(expected.promotions, result.promotions);

        return true;
    }

    // Test 1 - starting position
    REGISTER_TEST(movegen_starting_pos_test)
    {
        return test_generation("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                                2, {400, 0, 0, 0, 0}) &&
               test_generation("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                                3, {8902, 34, 0, 0, 0}) &&
               test_generation("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                                4, {197281, 1576, 0, 0, 0}) &&
               test_generation("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                                5, {4865609, 82719, 258, 0, 0});
    }

    // Test 2 - midgame position (standard)
    REGISTER_TEST(movegen_midgame_pos_standard_test)
    {
        return test_generation("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
                                1, {48, 8, 0, 2, 0}) &&
               test_generation("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
                                2, {2039, 351, 1, 91, 0}) &&
               test_generation("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
                                3, {97862, 17102, 45, 3162, 0}) &&
               test_generation("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
                                4, {4085603, 757163, 1929, 128013, 15172});
    }

    // Test 3 - midgame position (wild)
    REGISTER_TEST(movegen_midgame_pos_wild_test)
    {
        return test_generation("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
                                1, {6, 0, 0, 0, 0}) &&
               test_generation("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
                                2, {264, 87, 0, 6, 48}) &&
               test_generation("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
                                3, {9467, 1021, 4, 0, 120}) &&
               test_generation("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
                                4, {422333, 131393, 0, 7795, 60032});
    }


    // Test 4 - endgame position
    REGISTER_TEST(movegen_endgame_pos_test)
    {
        return test_generation("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
                                1, {14, 1, 0, 0, 0}) &&
               test_generation("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
                                2, {191, 14, 0, 0, 0}) &&
               test_generation("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
                                3, {2812, 209, 2, 0, 0}) &&
               test_generation("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
                                4, {43238, 3348, 123, 0, 0}) &&
               test_generation("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
                                5, {674624, 52051, 1165, 0, 0}) &&
               test_generation("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
                                6, {11030083, 940350, 33325, 0, 7552});
    }

}