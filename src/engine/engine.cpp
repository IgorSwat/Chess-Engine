#include "engine.h"
#include "searchconfig.h"
#include <chrono>


Search::Score Engine::evaluate(Search::Depth depth)
{
    // Step 1 - history malusement
    if (m_crawler.get_position()->halfmoves_p() < m_mem_board.halfmoves_p())
        m_history.reset();
    else
        m_history.flatten(std::max(1, m_crawler.get_position()->halfmoves_p() - m_mem_board.halfmoves_p()));

    // Step 2 - save current search position
    m_mem_board = *m_crawler.get_position();

    // Step 3 - main search
    Search::Score result = 0;

    // depth == 0 case is equivalent to evaluating the position statically
    if (depth == 0)
        result = Evaluation::relative_eval(m_crawler.search(0), *m_crawler.get_position());
    else if (m_mode == Engine::Mode::STANDARD)
        result = Evaluation::relative_eval(iterative_deepening(depth), *m_crawler.get_position());
    else if (m_mode == Engine::Mode::TRACE) {
        result = Evaluation::relative_eval(iterative_deepening(depth), *m_crawler.get_position());

        const Board* board = m_crawler.get_position();
        const TranspositionTable::Entry* entry = m_ttable.probe(board->hash(), board->pieces());

        std::cout << "\n|||||   Search results   |||||\n";
        std::cout << std::dec << "Score: " << Evaluation::relative_eval(entry->score, *board);
        std::cout << ", Type: " << int(m_crawler.m_search_stack[1].node) << 
                     ", Best move: " << m_crawler.m_search_stack[1].best_move << "\n";

        if (!entry) {
            std::cout << "Missing TT entry!!!\n";
        }
    }
    else {
        auto start = std::chrono::steady_clock::now();
        result = Evaluation::relative_eval(iterative_deepening(depth), *m_crawler.get_position());
        auto end = std::chrono::steady_clock::now();

        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << "Total search time: " << duration_ms.count() << " [ms]\n";
        std::cout << "> Nodes per second: " << (long long) (m_crawler.non_leaf_nodes + m_crawler.qs_nodes) * 1000 / duration_ms.count() << "\n";
        std::cout << "> Non-leaf nodes: " << m_crawler.non_leaf_nodes << "\n";
        std::cout << "> Leaf nodes: " << m_crawler.leaf_nodes << "\n";
        std::cout << "> Queiescence nodes: " << m_crawler.qs_nodes << "\n";
    }

    return result;
        
}

Search::Score Engine::iterative_deepening(Search::Depth depth)
{
    Search::Score result = 0;

    for (Search::Depth d = 1; d <= depth; d++)
        result = m_crawler.search(d);

    return result;
}