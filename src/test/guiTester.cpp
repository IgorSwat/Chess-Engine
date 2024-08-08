#include "guiTester.h"
#include <cstdlib>
#include <iomanip>


namespace Testing {

    // ---------------------
    // SearchPrinter methods
    // ---------------------

    SearchPrinter::SearchPrinter(Engine* engine, Search::Depth depth)
        : engine(engine), maxSearchDepth(depth)
    {
    }

    void SearchPrinter::initialTest(BoardConfig* board)
    {
        engine->setPosition(board);
        engine->evaluate(maxSearchDepth);
        nextTest(board);
    }

    void SearchPrinter::nextTest(BoardConfig* board)
    {
        system("clear");
        engine->setPosition(board);
        std::cout << "--- Static eval: " << std::dec << engine->evaluate(0) << "\n\n";
        std::cout << "--- Transposition table entries:\n";
        print_search_tree(board, engine->transpositionTable(), false);
        std::cout << std::endl;
    }

}