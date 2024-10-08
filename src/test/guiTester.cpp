#include "guiTester.h"
#include <cstdlib>
#include <iomanip>


namespace Testing {

    // ---------------------
    // SearchPrinter methods
    // ---------------------

    SearchPrinter::SearchPrinter(Engine* engine, bool useSearch, Search::Depth depth)
        : engine(engine), maxSearchDepth(depth), currSearchDepth(depth), useSearch(useSearch)
    {
    }

    void SearchPrinter::initialTest(BoardConfig* board)
    {
        currSearchDepth = maxSearchDepth + 1;
        nextTest(board, true);
    }

    void SearchPrinter::nextTest(BoardConfig* board, bool forward)
    {
        currSearchDepth += forward ? -1 : 1;
        currSearchDepth = std::min(currSearchDepth, static_cast<int>(maxSearchDepth));

        system("cls");
        engine->setPosition(board);
        std::cout << "--- Static eval: " << std::dec << engine->evaluate() << "\n\n";
        if (useSearch && currSearchDepth >= 0)
            engine->evaluate(static_cast<Search::Depth>(currSearchDepth));
    }

}