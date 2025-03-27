#pragma once

#include "board.h"
#include "history.h"
#include "moveord.h"
#include "nnue.h"
#include "searchconfig.h"
#include <algorithm>


/*
    ---------- Search ----------

    Definitions and implementations for engine's search algorithms
    - Decentralized search with Crawlers
*/

class Engine;
class TranspositionTable;

namespace Search {

    // -------------------------
    // Search - type definitions
    // -------------------------

    // To simplify some of search implementations, we allow depth to be a negative number
    using Depth = std::int8_t;

    // Equivalent of halfmoves plain for given position
    // - Since maximum length of chess game is around 6000 moves (12000 halfmoves), it's safe to use 16 bit integer
    using Age = std::int32_t;

    // Search score is basically an evaluation score of some position inside the search tree
    using Score = Eval;

    // Search node type
    // - There are 3 main categories of nodes: PV nodes (with exact score), cut nodes (beta-cutoff) and all nodes (no alpha increase)
    // - 3 most significant bits contain information about node category, ale other 5 allow to specialize subcategories
    enum Node : std::uint8_t {
        // Main categories
        PV_NODE = 1U << 7,
        CUT_NODE = 1U << 6,
        ALL_NODE = 1U << 5,

        // Subcategories
        ROOT_NODE = PV_NODE + 1,
        TERMINAL_NODE = PV_NODE + 2,    // A node where score is fixed (mate or stelmate found)

        // Other
        INVALID_NODE = 0,
        NON_PV_NODE = ALL_NODE - 1,
    };

    constexpr inline bool is_pv(Node node) { return node & PV_NODE; }
    constexpr inline bool is_cut(Node node) { return node & CUT_NODE; }
    constexpr inline bool is_all(Node node) { return node & ALL_NODE; }


    // -----------------
    // Search - crawlers
    // -----------------

    // Crawler is an object that performs search through given branch
    // - Main purpose is to simplify multithreading implementation of main search mechanism
    // - Contains individual (virtual board) and shared (transposition table & others) resources
    class Crawler
    {
    public:
        Crawler(TranspositionTable* ttable, History* history) : 
            m_ttable(ttable), m_history(history) { m_nnue.load("model/model_best.nnue"); }

        // Search
        // - This is only an API function - the biggest part of search implementation is packed inside helper functions
        Score search(Depth depth);

        // Static evaluation
        // - Since NNUE already returns a relative value, we do not need any additional conversion
        Eval evaluate() { return Evaluation::evaluate(m_virtual_board, m_nnue); }

        // Position setters
        void set_position(const std::string& fen) { m_virtual_board.load_position(fen); m_nnue.set(m_virtual_board); }
        void set_position(const Board& board) { m_virtual_board.load_position(board); m_nnue.set(m_virtual_board); }

        // Position getters
        const Board* get_position() const { return &m_virtual_board; }

        // [TESTING PURPOSES]
        // Last search data
        int non_leaf_nodes = 0;
        int leaf_nodes = 0;
        int qs_nodes = 0;

        friend class Engine;

    private:
        // Search components
        template <Node node>
        Score search(Score alpha, Score beta, Depth depth, bool nmp_available = false);
        template <Node node>
        Score quiescence(Score alpha, Score beta, Depth depth);

        // Helper functions - move make & unmake
        // - Making moves require changing different components in specyfic order, we want to add more abstraction with these methods
        // - They apply make/unmake on virtual board, increment/decrement search stack and update NNUE evaluator in appropriate way
        // - If only_stack = true, it does not apply changes to board or NNUE, just pushes search stack
        void make_move(const Move& move, bool only_stack = false);
        void undo_move();

        // Individual resources - virtual board
        Board m_virtual_board;

        // Individual resources - NNUE evaluator
        Evaluation::NNUE m_nnue;

        // Shared resources - transposition table connection
        TranspositionTable* m_ttable;

        // Shared resources - history table connection
        History* m_history;

        // Search stack implementation
        // - Search stack represents a single path in depth-first search algorithm
        // - Each node contains a relevant data for given ply
        // - ssTop is the head of the stack (represents most recently visited node)
        // - The first element is always a guard, which indicates moment we leave a search tree
        struct Ply
        {
            // Ply index - a non-negative integer from 0 up to maximum possible search (and quiescence) depth
            // - -1 is used as a default value, indicating the guard
            int16_t ply = -1;

            // Position age
            // - Related to ply index
            Age age = 0;

            // Evaluations
            // - static_eval always somces from NNUE evaluation, and eval is a value obtained from transposition table
            Eval static_eval = Evaluation::NO_EVAL;
            Eval eval = Evaluation::NO_EVAL;

            // Search data
            // - 3 main ingrediants of search result - score (evaluation), best move and type of node
            Depth depth = 0;
            Score score = -Evaluation::MAX_EVAL; 
            Move best_move = Moves::null;
            Node node = ALL_NODE;       // We can potentialy save 1 additional operation by setting ALL_NODE as default

            // Killer heuristic data
            Move killers[NO_KILLERS] = {};

            void add_killer(const Move& killer) {
                std::rotate(killers, killers + NO_KILLERS - 1, killers + NO_KILLERS);
                killers[0] = killer;
            }
            
            // LMR heuristic data
            // - If move is the first one tried at given search node, then move_idx = 1, if 2nd, then move_idx = 2, etc.
            // - Decides about size of reduction in LMR heuristic
            int move_idx = 0;
        };

        // Search stack and pointer
        Ply m_search_stack[MAX_TOTAL_SEARCH_DEPTH + 1] = {};
        Ply* m_sstop;

        // LMR flag
        bool m_use_lmr = false;
    };

}