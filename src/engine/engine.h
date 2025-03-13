#pragma once

#include "search.h"
#include "ttable.h"


/*
    ---------- Engine ----------

    Implementation of main engine class
*/

class Engine
{
public:

    // Engine mode definition
    // - Allows for different behavior in debug, test and real usages of the engine
    enum class Mode {
        STANDARD = 0,
        TRACE,
        STATS
    };

    Engine(Mode mode) : m_mode(mode), m_crawler(&m_ttable, &m_history) {}

    // Setup
    void set_position(const Board& board) { m_crawler.set_position(board); }
    void set_position(const std::string& fen) { m_crawler.set_position(fen); }

    // Main functionalities
    Search::Score evaluate(Search::Depth depth = 0);
    Search::Score iterative_deepening(Search::Depth depth);

    // Getters
    const TranspositionTable* ttable() const { return &m_ttable; }

private:
    // Engine mode
    Mode m_mode;

    // Shared modules
    TranspositionTable m_ttable;
    Search::History m_history;

    // Search crawlers
    Search::Crawler m_crawler;    // For now we use 1 crawler as we use only 1 thread on search

    // Search position snapshot
    Board m_mem_board;
};