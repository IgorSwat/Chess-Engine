#include "test.h"
#include "../engine/engine.h"
#include <fstream>
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
                os << offset << int(depth) << ": " << move << std::dec << " ... Score: " << entry->score << ", Type: " << int(entry->typeOfNode);
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
        Search::Depth depth = 3;

        BoardConfig board;
        board.loadPosition(fen);

        std::unique_ptr<Engine> engine = std::make_unique<Engine>();
        engine->setPosition(&board);
        engine->evaluate(depth);

        print_search_tree(&board, engine->transpositionTable(), true, OutputMode::CONSOLE);
    }

}