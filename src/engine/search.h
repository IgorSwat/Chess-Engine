#pragma once

#include "moveSelection.h"
#include "evaluation.h"
#include "searchConfig.h"
#include <numeric>


// For pointers inside crawler class
class TranspositionTable;


namespace Search {

    // ----------------
    // Type definitions
    // ----------------

    using Depth = std::uint8_t;
    using Age = std::uint16_t;

    // Node (position) type info
     enum NodeType : std::uint8_t {
        PV_NODE = 1,
        ROOT_NODE = (PV_NODE << 1) | PV_NODE,           // Exact value
        CUT_NODE = PV_NODE << 2,                        // A lower-bound score (a line which should not be allowed by enemy)
        ALL_NODE = PV_NODE << 3,                        // An upper-bound score (no move exceeds alpha - best possible score for moving side)
        NON_PV_NODE = PV_NODE << 4,                     // Either root, cut or all node
        TERMINAL_NODE = (PV_NODE << 4) | PV_NODE,       // A node where checkmate or stealmate appears

        INVALID_NODE = 0,
    };

    constexpr Value MAX_EVAL = 1e4;   // Used as a checkmate evaluation and upper boundary for beta
    constexpr Value NO_EVAL = MAX_EVAL - 1;


    // ----------------
    // Helper functions
    // ----------------

    inline Age distance(Age rootAge, Age age)
    {
        return age > rootAge ? age - rootAge : rootAge - age;
    }

    inline Value relative_score(Value score, const BoardConfig* board)
    {
        return board->movingSide() == WHITE ? score : -score;
    }


    // -------------
    // Crawler class
    // -------------

    // Main search mechanism
    class Crawler
    {
    public:
        Crawler(TranspositionTable* tTable) 
            : virtualBoard(), evaluator(&this->virtualBoard), tTable(tTable), ssTop(searchStack) {}

        // Setup
        void setPosition(BoardConfig* board);
        void setPosition(const std::string& fen);
        const BoardConfig* getPosition() const { return &virtualBoard; }

        // Main search functions
        Value search(Depth depth);                              // Initial function

        // Last search data
        int non_leaf_nodes = 0;
        int leaf_nodes = 0;
        int qs_nodes = 0;

        // Extra search info
        struct SearchInfo
        {
            int ply = -1;
            Value staticEval = NO_EVAL;
            Value eval = NO_EVAL;

            Value score = -MAX_EVAL;
            NodeType node = ALL_NODE;
            Move bestMove = Move::null();
        };

    private:
        // Search helpers
        Value evaluate();                                       // Stand pat evaluation = search on depth 0
        template <NodeType stage, bool nmpAvailable>            // NMP = Null Move Pruning
        Value search(Value alpha, Value beta, Depth depth);     // Recursive subroutine
        template <NodeType stage>
        Value quiescence(Value alpha, Value beta, Depth depth);

        // Search stack handlers
        void pushStack();

        // Virtual board and related properties
        BoardConfig virtualBoard;
        Age rootAge = 0;

        // Additional search info
        SearchInfo searchStack[MAX_SEARCH_DEPTH + MAX_QUIESCENCE_DEPTH + 1] = {};
        SearchInfo* ssTop;

        // Evaluation helper
        Evaluation::Evaluator evaluator;

        // Transposition table lookup
        TranspositionTable* tTable;
    };


    // ---------------
    // Crawler methods
    // ---------------

    inline void Crawler::setPosition(BoardConfig* board)
    {
        virtualBoard.loadPosition(*board);
        rootAge = virtualBoard.halfmovesPlain();
    }

    inline void Crawler::setPosition(const std::string& fen)
    {
        virtualBoard.loadPosition(fen);
        rootAge = virtualBoard.halfmovesPlain();
    }

    inline Value Crawler::evaluate()
    {
        return relative_score(evaluator.evaluate(), &virtualBoard);
    }

    inline void Crawler::pushStack()
    {
        ++ssTop;
        ssTop->ply = (ssTop - 1)->ply + 1;
        ssTop->score = -MAX_EVAL;
        ssTop->node = ALL_NODE;
        ssTop->eval = ssTop->staticEval = NO_EVAL;
        ssTop->bestMove = Move::null();
    }

}