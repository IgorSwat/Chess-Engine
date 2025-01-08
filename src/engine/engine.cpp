#include "engine.h"
#include "searchConfig.h"
#include <chrono>


// --------------
// Engine methods
// --------------

void Engine::setPosition(BoardConfig* board)
{
    crawler.setPosition(board);

    // Reset shared resources
    history.reset();
    //history.flatten();
}

void Engine::setPosition(const std::string& fen)
{
    crawler.setPosition(fen);

    // Reset shared resources
    history.reset();
    //history.flatten();
}

// Returns a relative score - to transform it back to absolute score you need to apply relative_score() again
Value Engine::evaluate(Search::Depth depth)
{
    if (depth == 0)
        return Search::relative_score(crawler.search(0), crawler.getPosition());

    if constexpr (SEARCH_MODE == STANDARD)
        return iterativeDeepening(depth);
    
    if constexpr (SEARCH_MODE == TRACE) {
        Value score = crawler.search(depth);

        const BoardConfig* board = crawler.getPosition();
        const TranspositionTable::Entry* entry = tTable.probe(board->hash(), board->pieces());

        if (entry) {
            std::cout << "Hash: " << entry->key << ", pieces: " << entry->pieces << "\n\n";
            std::cout << "\n|||||   Search results   |||||\n";
            std::cout << std::dec << "Score: " << Search::relative_score(entry->score, board);
            std::cout << ", Type: " << int(entry->typeOfNode) << ", Best move: " << entry->bestMove << "\n";
        }

        // showHistory();

        return score;
    }
    
    if constexpr (SEARCH_MODE == STATS) {
        auto start = std::chrono::steady_clock::now();
        Value result = iterativeDeepening(depth);
        auto end = std::chrono::steady_clock::now();

        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cout << "Total search time: " << duration_ms.count() << " [ms]\n";
        std::cout << "> Nodes per second: " << (long long) (crawler.non_leaf_nodes + crawler.qs_nodes) * 1000 / duration_ms.count() << "\n";
        std::cout << "> Non-leaf nodes: " << crawler.non_leaf_nodes << "\n";
        std::cout << "> Leaf nodes: " << crawler.leaf_nodes << "\n";
        std::cout << "> Queiescence nodes: " << crawler.qs_nodes << "\n";

        return result;
    }
}

Value Engine::iterativeDeepening(Search::Depth depth)
{
    Value result = 0;
    for (Search::Depth d = 1; d <= depth; d++)
        result = crawler.search(d);
    return result;
}

const TranspositionTable* Engine::transpositionTable() const
{
    return &tTable;
}

// TMP
void Engine::showHistory() const
{
    for (int i = 0; i < PIECE_RANGE; i++) {
        std::cout << "Piece " << i << ":\n";
        for (int sq = 0; sq < SQUARE_RANGE; sq++) {
            if (history.score(Piece(i), Square(sq)) != 0)
                std::cout << "\t" << Square(sq) << ": " << history.score(Piece(i), Square(sq)) << "\n";
        }
    }
}