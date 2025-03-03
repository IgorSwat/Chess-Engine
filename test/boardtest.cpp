#include "test.h"
#include "../src/engine/board.h"
#include <vector>


namespace Testing {

    // ---------------------------
    // Board test - static loading
    // ---------------------------

    // The primary goal is to make sure that both methods of static loading (from FEM and another board) produce the same,
    // correct result
    // - Very important test - most of the following tests relie on the correctness of FEN loading
    REGISTER_TEST(board_static_load_test)
    {
        // It's important to select position which covers all special aspects of static load (castling, enpassant, move counting)
        const std::string fen = "r2qkb1r/1p4pp/2n1b1p1/pBP1Pp2/1P5R/n3PNP1/P2N1B1P/R1Q1K3 w Qkq f6 0 7";

        // Here we test two ways of static loading - by parsing FEN and by copying other board
        // - First value of each pair is the name of loading method, which allows to identify cases
        std::vector<std::pair<std::string, Board>> cases = {
            {"FEN", Board()}, 
            {"COPY", Board()}
        };

        cases[0].second.load_position(fen);
        cases[1].second.load_position(cases[0].second);

        // Test two boards simultaneously in the same way
        for (const auto& tcase : cases) {
            const Board& board = tcase.second;

            ASSERT_EQUALS(WHITE, board.side_to_move());

            ASSERT_EQUALS(0xb9c254378271a915, board.pieces());
            ASSERT_EQUALS(0x000000168270a915, board.pieces(WHITE));
            ASSERT_EQUALS(0xb9c2542100010000, board.pieces(BLACK));

            ASSERT_EQUALS(0x0000001402508100, board.pieces(WHITE, PAWN));
            ASSERT_EQUALS(0x0000000200202800, board.pieces(WHITE, KNIGHT, BISHOP));
            ASSERT_EQUALS(0x0000000080000005, board.pieces(WHITE, ROOK, QUEEN));
            ASSERT_EQUALS(SQ_E1, board.king_position(WHITE));

            ASSERT_EQUALS(0x00c2402100000000, board.pieces(BLACK, PAWN));
            ASSERT_EQUALS(0x2000140000010000, board.pieces(BLACK, KNIGHT, BISHOP));
            ASSERT_EQUALS(0x8900000000000000, board.pieces(BLACK, ROOK, QUEEN));
            ASSERT_EQUALS(SQ_E8, board.king_position(BLACK));

            ASSERT_EQUALS(0xe, board.castling_rights());
            ASSERT_EQUALS(false, board.can_castle(WHITE, KINGSIDE_CASTLE));
            ASSERT_EQUALS(true, board.can_castle(WHITE, QUEENSIDE_CASTLE));
            ASSERT_EQUALS(true, board.can_castle(BLACK, KINGSIDE_CASTLE));
            ASSERT_EQUALS(true, board.can_castle(BLACK, QUEENSIDE_CASTLE));
            ASSERT_EQUALS(true, board.castle_path_clear(WHITE, KINGSIDE_CASTLE));
            ASSERT_EQUALS(false, board.castle_path_clear(WHITE, QUEENSIDE_CASTLE));
            ASSERT_EQUALS(false, board.castle_path_clear(BLACK, KINGSIDE_CASTLE));
            ASSERT_EQUALS(false, board.castle_path_clear(BLACK, QUEENSIDE_CASTLE));

            ASSERT_EQUALS(SQ_F5, board.enpassant_square());

            ASSERT_EQUALS(0, board.halfmoves_c());

            ASSERT_EQUALS(0x0000040000000000, board.pinned(BLACK));
            ASSERT_EQUALS(0x0000000200000000, board.pinners(WHITE));

            std::cout << "- " << tcase.first << " loading: CORRECT\n";
        }

        return true;
    }


    // ------------------------------------------------
    // Board test - dynamic change (move make & unmake)
    // ------------------------------------------------

    // To compose next tests, we make use of the following facts:
    // - Static loading is most likely correct (since we already have a test for that)
    // - We can have 2 separate boards: one updated dynamically, and the other one updated statically (as a point of reference)

    // Test 1 - from starting position into some Ruy Lopez lines
    REGISTER_TEST(board_dynamic_change_test_1)
    {
        // Here we can hardcode moves and the resulting position in FEN notation
        const std::vector<std::pair<Move, std::string>> progress = {
            std::make_pair(Move(SQ_E2, SQ_E4, Moves::DOUBLE_PAWN_PUSH_FLAG), 
                           "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1"),
            std::make_pair(Move(SQ_E7, SQ_E5, Moves::DOUBLE_PAWN_PUSH_FLAG), 
                           "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2"),
            std::make_pair(Move(SQ_G1, SQ_F3, Moves::QUIET_MOVE_FLAG), 
                           "rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2"),
            std::make_pair(Move(SQ_B8, SQ_C6, Moves::QUIET_MOVE_FLAG), 
                           "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3"),
            std::make_pair(Move(SQ_F1, SQ_B5, Moves::QUIET_MOVE_FLAG), 
                           "r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3"),
            std::make_pair(Move(SQ_G8, SQ_F6, Moves::QUIET_MOVE_FLAG), 
                           "r1bqkb1r/pppp1ppp/2n2n2/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4"),
            std::make_pair(Move(SQ_E1, SQ_G1, Moves::KINGSIDE_CASTLE_FLAG), 
                           "r1bqkb1r/pppp1ppp/2n2n2/1B2p3/4P3/5N2/PPPP1PPP/RNBQ1RK1 b kq - 5 4"),
            std::make_pair(Move(SQ_F6, SQ_E4, Moves::CAPTURE_FLAG), 
                           "r1bqkb1r/pppp1ppp/2n5/1B2p3/4n3/5N2/PPPP1PPP/RNBQ1RK1 w kq - 0 5"),
            std::make_pair(Move(SQ_D2, SQ_D4, Moves::DOUBLE_PAWN_PUSH_FLAG), 
                           "r1bqkb1r/pppp1ppp/2n5/1B2p3/3Pn3/5N2/PPP2PPP/RNBQ1RK1 b kq - 0 5"),
            std::make_pair(Move(SQ_E4, SQ_D6, Moves::QUIET_MOVE_FLAG), 
                           "r1bqkb1r/pppp1ppp/2nn4/1B2p3/3P4/5N2/PPP2PPP/RNBQ1RK1 w kq - 1 6"),
            std::make_pair(Move(SQ_B5, SQ_C6, Moves::CAPTURE_FLAG), 
                           "r1bqkb1r/pppp1ppp/2Bn4/4p3/3P4/5N2/PPP2PPP/RNBQ1RK1 b kq - 0 6"),
            std::make_pair(Move(SQ_D7, SQ_C6, Moves::CAPTURE_FLAG), 
                           "r1bqkb1r/ppp2ppp/2pn4/4p3/3P4/5N2/PPP2PPP/RNBQ1RK1 w kq - 0 7"),
            std::make_pair(Move(SQ_D4, SQ_E5, Moves::CAPTURE_FLAG), 
                           "r1bqkb1r/ppp2ppp/2pn4/4P3/8/5N2/PPP2PPP/RNBQ1RK1 b kq - 0 7"),
            std::make_pair(Move(SQ_D6, SQ_F5, Moves::QUIET_MOVE_FLAG), 
                           "r1bqkb1r/ppp2ppp/2p5/4Pn2/8/5N2/PPP2PPP/RNBQ1RK1 w kq - 1 8"),
            std::make_pair(Move(SQ_D1, SQ_D8, Moves::CAPTURE_FLAG), 
                           "r1bQkb1r/ppp2ppp/2p5/4Pn2/8/5N2/PPP2PPP/RNB2RK1 b kq - 0 8"),
            std::make_pair(Move(SQ_E8, SQ_D8, Moves::CAPTURE_FLAG), 
                           "r1bk1b1r/ppp2ppp/2p5/4Pn2/8/5N2/PPP2PPP/RNB2RK1 w - - 0 9"),
            std::make_pair(Move(SQ_F1, SQ_D1, Moves::QUIET_MOVE_FLAG), 
                           "r1bk1b1r/ppp2ppp/2p5/4Pn2/8/5N2/PPP2PPP/RNBR2K1 b - - 1 9"),
            std::make_pair(Move(SQ_C8, SQ_D7, Moves::QUIET_MOVE_FLAG), 
                           "r2k1b1r/pppb1ppp/2p5/4Pn2/8/5N2/PPP2PPP/RNBR2K1 w - - 2 10"),
        };

        // Two boards - one as testing board, and the other as a reference
        Board test_board, reference_board;

        // Forward pass - testing move makers
        for (auto it = progress.begin(); it != progress.end(); it++) {
            test_board.make_move(it->first);
            reference_board.load_position(it->second);

            ASSERT_EQUALS(reference_board, test_board);

            // Let's additionally test halfmove clock and zobrist hash
            ASSERT_EQUALS(reference_board.halfmoves_c(), test_board.halfmoves_c());
            ASSERT_EQUALS(reference_board.hash(), test_board.hash());
        }

        // Backward pass - testing move unmake
        for (auto it = progress.rbegin(); it != progress.rend(); it++) {
            reference_board.load_position(it->second);

            ASSERT_EQUALS(reference_board, test_board);

            test_board.undo_move();
        }

        return true;
    }

    // Test 2 - from custom midgame position
    REGISTER_TEST(board_dynamic_change_test_2) 
    {
        // Position
        const std::string fen = "4r1k1/Ppp1np2/p5pp/q2P4/7b/2P2P2/P1Q3P1/R3K1BN w Q - 0 1";

        const std::vector<std::pair<Move, std::string>> progress = {
            std::make_pair(Move(SQ_H1, SQ_F2, Moves::QUIET_MOVE_FLAG),
                           "4r1k1/Ppp1np2/p5pp/q2P4/7b/2P2P2/P1Q2NP1/R3K1B1 b Q - 1 1"),
            std::make_pair(Move(SQ_H4, SQ_F2, Moves::CAPTURE_FLAG),
                           "4r1k1/Ppp1np2/p5pp/q2P4/8/2P2P2/P1Q2bP1/R3K1B1 w Q - 0 2"),
            std::make_pair(Move(SQ_G1, SQ_F2, Moves::CAPTURE_FLAG),
                           "4r1k1/Ppp1np2/p5pp/q2P4/8/2P2P2/P1Q2BP1/R3K3 b Q - 0 2"),
            std::make_pair(Move(SQ_E8, SQ_D8, Moves::QUIET_MOVE_FLAG),
                           "3r2k1/Ppp1np2/p5pp/q2P4/8/2P2P2/P1Q2BP1/R3K3 w Q - 1 3"),
            std::make_pair(Move(SQ_E1, SQ_C1, Moves::QUEENSIDE_CASTLE_FLAG),
                           "3r2k1/Ppp1np2/p5pp/q2P4/8/2P2P2/P1Q2BP1/2KR4 b - - 2 3"),
            std::make_pair(Move(SQ_D8, SQ_D7, Moves::QUIET_MOVE_FLAG),
                           "6k1/Ppprnp2/p5pp/q2P4/8/2P2P2/P1Q2BP1/2KR4 w - - 3 4"),
            std::make_pair(Move(SQ_A7, SQ_A8, Moves::QUEEN_PROMOTION_FLAG),
                           "Q5k1/1pprnp2/p5pp/q2P4/8/2P2P2/P1Q2BP1/2KR4 b - - 0 4"),
            std::make_pair(Move(SQ_E7, SQ_C8, Moves::QUIET_MOVE_FLAG),
                           "Q1n3k1/1ppr1p2/p5pp/q2P4/8/2P2P2/P1Q2BP1/2KR4 w - - 1 5"),
            std::make_pair(Move(SQ_A8, SQ_C8, Moves::CAPTURE_FLAG),
                           "2Q3k1/1ppr1p2/p5pp/q2P4/8/2P2P2/P1Q2BP1/2KR4 b - - 0 5"),
            std::make_pair(Move(SQ_G8, SQ_G7, Moves::QUIET_MOVE_FLAG),
                           "2Q5/1ppr1pk1/p5pp/q2P4/8/2P2P2/P1Q2BP1/2KR4 w - - 1 6"),
        };

        Board test_board, reference_board;
        test_board.load_position(fen);
        reference_board.load_position(fen);

        // Forward pass - testing move makers
        for (auto it = progress.begin(); it != progress.end(); it++) {
            test_board.make_move(it->first);
            reference_board.load_position(it->second);

            ASSERT_EQUALS(reference_board, test_board);

            // Let's additionally test halfmove clock and zobrist hash
            ASSERT_EQUALS(reference_board.halfmoves_c(), test_board.halfmoves_c());
            ASSERT_EQUALS(reference_board.hash(), test_board.hash());
        }

        // Backward pass - testing move unmake
        for (auto it = progress.rbegin(); it != progress.rend(); it++) {
            reference_board.load_position(it->second);

            ASSERT_EQUALS(reference_board, test_board);

            test_board.undo_move();
        }

        return true;
    }


    // ---------------------------
    // Board test - null move test
    // ---------------------------

    // A proper work of null move making & unmaking plays a crucial role in one of main selectivity features - Null Move Pruning
    REGISTER_TEST(board_null_move_test)
    {
        // First element of each pair is a random position FEN, and the second one is notation a after passing move
        const std::vector<std::pair<std::string, std::string>> positions = {
            {"r1bq1b1r/pp4pp/2p1k1n1/3np3/2BP4/2N2Q2/PPP2PPP/R1B2RK1 w - - 2 11",
             "r1bq1b1r/pp4pp/2p1k1n1/3np3/2BP4/2N2Q2/PPP2PPP/R1B2RK1 b - - 2 11"},
            {"rnbq1rk1/ppp2ppp/5n2/2b1N3/2B1P3/8/PPPP2PP/RNBQK2R w KQ - 1 7",
             "rnbq1rk1/ppp2ppp/5n2/2b1N3/2B1P3/8/PPPP2PP/RNBQK2R b KQ - 1 7"},
            {"r2qkbnr/pbp1pppp/n7/1p1pP2Q/8/P7/1PPP1PPP/RNB1KBNR w KQkq d6 0 5",
             "r2qkbnr/pbp1pppp/n7/1p1pP2Q/8/P7/1PPP1PPP/RNB1KBNR b KQkq - 0 5"},
            {"6k1/1q2bppp/5n2/1P1p4/P2Bp3/3bP1P1/2QN1PBP/R5K1 b - - 0 30",
             "6k1/1q2bppp/5n2/1P1p4/P2Bp3/3bP1P1/2QN1PBP/R5K1 w - - 0 30"},
            {"3r1r2/5pk1/p1b1p1p1/1pP4p/3NpP2/2P1P2R/P2q2PP/R2Q2K1 b - - 1 23",
             "3r1r2/5pk1/p1b1p1p1/1pP4p/3NpP2/2P1P2R/P2q2PP/R2Q2K1 w - - 1 23"}
        };

        Board test_board, reference_board;

        // The idea behind this test is simple - load some random position, make null move, test with reference board, unmake null move
        // and test it again

        for (const auto& pos : positions) {
            test_board.load_position(pos.first);
            test_board.make_null_move();
            reference_board.load_position(pos.second);

            ASSERT_EQUALS(reference_board, test_board);

            test_board.undo_null_move();
            reference_board.load_position(pos.first);

            ASSERT_EQUALS(reference_board, test_board);
        }

        return true;
    }


    // -------------------------------------
    // Board test - repetition counting test
    // -------------------------------------

    REGISTER_TEST(board_repetition_count_test)
    {
        const std::string fen = "8/6bp/p5p1/1pk5/3p1P2/5KP1/P1P2B1P/8 w - - 0 35";

        Board board;
        board.load_position(fen);

        ASSERT_EQUALS(1, board.repetitions());

        board.make_move(Move(SQ_F3, SQ_E4, Moves::QUIET_MOVE_FLAG));
        board.make_move(Move(SQ_C5, SQ_C6, Moves::QUIET_MOVE_FLAG));
        board.make_move(Move(SQ_E4, SQ_F3, Moves::QUIET_MOVE_FLAG));

        ASSERT_EQUALS(1, board.repetitions());

        board.make_move(Move(SQ_C6, SQ_C5, Moves::QUIET_MOVE_FLAG));

        ASSERT_EQUALS(2, board.repetitions());

        board.make_move(Move(SQ_F3, SQ_E4, Moves::QUIET_MOVE_FLAG));

        ASSERT_EQUALS(2, board.repetitions());

        board.make_move(Move(SQ_C5, SQ_B6, Moves::QUIET_MOVE_FLAG));
        board.make_move(Move(SQ_E4, SQ_D3, Moves::QUIET_MOVE_FLAG));
        board.make_move(Move(SQ_B6, SQ_C6, Moves::QUIET_MOVE_FLAG));
        board.make_move(Move(SQ_D3, SQ_E4, Moves::QUIET_MOVE_FLAG));

        ASSERT_EQUALS(1, board.repetitions());

        board.make_move(Move(SQ_C6, SQ_D6, Moves::QUIET_MOVE_FLAG));
        board.make_move(Move(SQ_E4, SQ_D3, Moves::QUIET_MOVE_FLAG));
        board.make_move(Move(SQ_D6, SQ_D5, Moves::QUIET_MOVE_FLAG));
        board.make_move(Move(SQ_D3, SQ_E2, Moves::QUIET_MOVE_FLAG));
        board.make_move(Move(SQ_D5, SQ_C6, Moves::QUIET_MOVE_FLAG));
        board.make_move(Move(SQ_H2, SQ_H3, Moves::QUIET_MOVE_FLAG));
        board.make_move(Move(SQ_C6, SQ_B6, Moves::QUIET_MOVE_FLAG));
        board.make_move(Move(SQ_E2, SQ_F3, Moves::QUIET_MOVE_FLAG));
        board.make_move(Move(SQ_B6, SQ_C5, Moves::QUIET_MOVE_FLAG));

        ASSERT_EQUALS(1, board.repetitions());

        return true;
    }


    // --------------------------------------
    // Board test - move legality checks test
    // --------------------------------------

    // They should be fine for now...
    // TODO: add in the future

}