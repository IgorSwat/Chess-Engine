#pragma once

#include "test.h"


namespace Testing {

    // -------------------
    // GuiTester interface
    // -------------------

    // A abstract class which defines a framework to interact with BoardController and
    // perform some test, validation or create logs after every change to virtual board
    class GuiTester
    {
    public:
        virtual ~GuiTester() {}
        virtual void initialTest(BoardConfig* board) = 0;
        virtual void nextTest(BoardConfig* board) = 0;
    };


    // -------------------
    // SearchPrinter class
    // -------------------

    // Performs search on given maximal depth
    // Prints current position static eval as well as related results from transposition table
    class SearchPrinter : public GuiTester
    {
    public:
        SearchPrinter(Engine* engine, Search::Depth depth);

        void initialTest(BoardConfig* board) override;
        void nextTest(BoardConfig* board) override;

    private:
        Engine* engine;
        Search::Depth maxSearchDepth;
    };

}