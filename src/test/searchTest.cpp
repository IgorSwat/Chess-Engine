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

    void printSearchTree(BoardConfig* board, const TranspositionTable* tTable, Search::Depth depth, std::ostream& os)
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
                printSearchTree(board, tTable, depth + 1, os);
            }

            board->undoLastMove();
        }
    }

    template <OutputMode mode>
    void printSearchTree(BoardConfig* board, const TranspositionTable* tTable)
    {
        // Select output object
        std::ostream& out = std::cout;
        if constexpr (mode == OutputMode::FILE) {
            std::ofstream file("search_test.txt");
            out = file;
        }

        printSearchTree(board, tTable, 0, out);
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

        printSearchTree<OutputMode::CONSOLE>(&board, engine->transpositionTable());
    }

}