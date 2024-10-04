#include "test.h"
#include "../engine/engine.h"
#include <fstream>
#include <chrono>
#include <memory>
#include <vector>
#include <algorithm>


namespace Testing {

    // ------------
    // Test helpers
    // ------------

    void print_search_tree(BoardConfig* board, const TranspositionTable* tTable, Search::Depth depth, bool deep, std::ostream& os)
    {
        // Generate all legal moves
        MoveList moves;
        MoveGeneration::generate_moves<MoveGeneration::LEGAL>(*board, moves);

        for (const Move& move : moves) {
            board->makeMove(move);

            auto entry = tTable->probe(board->hash(), board->pieces());
            if (entry != nullptr) {
                std::string offset(2 * depth, ' ');
                os << offset << int(depth) << ": " << move << std::dec << " ... Score: " << Search::relative_score(entry->score, board);
                os << ", Type: " << int(entry->typeOfNode);
                os << ", ((( Best " << entry->bestMove << " )))\n";
                if (deep)
                    print_search_tree(board, tTable, depth + 1, deep, os);
            }

            board->undoLastMove();
        }
    }

    void print_search_tree(BoardConfig* board, const TranspositionTable* tTable, bool deep, OutputMode mode)
    {
        // Select output object
        if (mode == OutputMode::FILE) {
            std::ofstream file("search_test.txt");
            print_search_tree(board, tTable, 0, deep, file);
        }
        else
            print_search_tree(board, tTable, 0, deep, std::cout);
    }


    // --------------
    // Test functions
    // --------------

    void searchTest1()
    {
        std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        //std::string fen = "3r1rk1/pb3pbp/1pn5/2p5/4n3/2P4P/PPB2PPN/R1B2RK1 b - - 1 18";
        Search::Depth depth = 8;

        BoardConfig board;
        board.loadPosition(fen);

        std::unique_ptr<Engine> engine = std::make_unique<Engine>();
        engine->setPosition(&board);

        auto start = std::chrono::steady_clock::now();
        engine->evaluate(depth);
        //engine->iterativeDeepening(depth);
        auto end = std::chrono::steady_clock::now();

        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Search (depth " << std::dec << int(depth) << ") time: " << elapsed.count() << " seconds" << std::endl;

        //print_search_tree(&board, engine->transpositionTable(), true, OutputMode::CONSOLE);
    }

    void searchMultiTest()
    {
        BoardConfig board;
        std::unique_ptr<Engine> engine = std::make_unique<Engine>();

        std::vector<std::string> positions = {
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",                 // Starting position
            "1r1q1rk1/3bppbp/3p2pB/1np2P2/p2nP1P1/2NP1N1P/PP1Q2B1/1R3RK1 w - - 3 19",    // Midgame position, complex
            "rn1q1rk1/ppp1bpp1/5n1p/4p1P1/4p1bP/1PN5/PBPPQP2/2KR1BNR w - - 1 10",       // Midgame position, complex, weird things with sorting
            "R7/8/4k3/3n4/1p4P1/5P2/5K2/8 w - - 0 51",                                  // Endgame position, simple
            "8/4kbp1/5p2/5Q1p/8/8/5K2/8 w - - 1 51",                                    // Endgame position, complex
        };

        const Search::Depth depth = 7;

        int id = 0;
        for (const std::string& fen : positions) {
            id++;

            board.loadPosition(fen);
            engine->setPosition(&board);

            std::cout << "----- Position " << id << " -----\n";
            engine->evaluate(depth);
            std::cout << "\n";
        }
    }

}