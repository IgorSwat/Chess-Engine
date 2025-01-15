#include "test.h"
#include "../engine/engine.h"
#include "../utilities/parsing.h"
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
            "r1bq2k1/ppppbrpp/8/4Pp1Q/4pB2/2N5/PPP2PPP/R3R1K1 w - - 2 15",              // Midgame position, calm
            "1r1q1rk1/3bppbp/3p2pB/1np2P2/p2nP1P1/2NP1N1P/PP1Q2B1/1R3RK1 w - - 3 19",   // Midgame position, complex
            "rn1q1rk1/ppp1bpp1/5n1p/4p1P1/4p1bP/1PN5/PBPPQP2/2KR1BNR w - - 1 10",       // Midgame position, complex, weird things with sorting
            "R7/8/4k3/3n4/1p4P1/5P2/5K2/8 w - - 0 51",                                  // Endgame position, simple
            "8/4kbp1/5p2/5Q1p/8/8/5K2/8 w - - 1 51",                                    // Endgame position, complex
        };

        const Search::Depth depth = 11;

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

    void search_quality_test()
    {
        // Each case is a (position, moves) pair, where moves is a list of moves and points associated with them
        // Engine can score max 10 points for pointing out given move as the best in the position
        // If best move is not mentioned on appropriate list, then it scores 0 points

        BoardConfig board;
        std::unique_ptr<Engine> engine = std::make_unique<Engine>();
        
        std::ifstream file("testpos/search-quality-test.txt");

        const Search::Depth depth = 11;

        int totalScore = 0, maxPossibleScore = 0, totalTests = 0;
        std::chrono::duration<double> totalSearchTime;

        while (!file.eof()) {
            totalTests++;
            maxPossibleScore += 10;

            int noMoves;
            file >> noMoves;

            std::string fen;
            file.get();
            std::getline(file, fen);

            board.loadPosition(fen);
            engine->setPosition(&board);

            auto start = std::chrono::steady_clock::now();
            engine->evaluate(depth);
            auto end = std::chrono::steady_clock::now();

            const TranspositionTable::Entry* entry = engine->transpositionTable()->probe(board.hash(), board.pieces());

            if (entry == nullptr)
                maxPossibleScore -= 10;
            else
                totalSearchTime += (end - start);

            std::string moveNotation;
            int score;
            bool accepted = false;

            for (int i = 0; i < noMoves; i++) {
                file >> moveNotation;
                file >> score;
                
                Move move = Utilities::Parsing::parse_move(board, moveNotation);

                if (i == 0 && entry != nullptr)
                    std::cout << std::dec << "Test " << totalTests << "  |  best: " << move << "  |  found: " << entry->bestMove << "\n";
                else if (i == 0 && entry == nullptr)
                    std::cout << std::dec << "Test " << totalTests << ": no transposition table entry available\n";

                if (!accepted && entry != nullptr && move == entry->bestMove) {
                    totalScore += score;
                    accepted = true;
                }
            }
        }

        std::cout << std::dec << "---------------------------------------------------\n";
        std::cout << "Test cases: " << totalTests << "\n";
        std::cout << "Total score: " << totalScore << "/" << maxPossibleScore << " (" << totalScore * 100 / maxPossibleScore << "%)\n";
        std::cout << "Total search time: " << totalSearchTime.count() << " seconds \n";
    }

}