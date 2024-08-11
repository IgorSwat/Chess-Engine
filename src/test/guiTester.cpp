#include "guiTester.h"
#include <cstdlib>
#include <iomanip>


namespace Testing {

    // ---------------------
    // SearchPrinter methods
    // ---------------------

    SearchPrinter::SearchPrinter(Engine* engine, bool useSearch, Search::Depth depth)
        : engine(engine), maxSearchDepth(depth), useSearch(useSearch)
    {
    }

    void SearchPrinter::initialTest(BoardConfig* board)
    {
        engine->setPosition(board);
        if (useSearch)
            engine->evaluate(maxSearchDepth);
        nextTest(board);
    }

    void SearchPrinter::nextTest(BoardConfig* board)
    {
        system("clear");
        engine->setPosition(board);
        std::cout << "--- Static eval: " << std::dec << Search::relative_score(engine->evaluate(0), board) << "\n";
        if (useSearch) {
            std::cout << "\n--- Transposition table entries:\n";
            print_search_tree(board, engine->transpositionTable(), false);
            std::cout << "\n";
        }
    }

}