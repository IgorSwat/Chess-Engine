#pragma once

#include "moveSelection.h"
#include "evaluation.h"
#include "searchConfig.h"
#include <algorithm>
#include <numeric>


class TranspositionTable;

namespace Search {

    // ----------------
    // Type definitions
    // ----------------

    using Depth = std::uint8_t;
    using Age = std::uint16_t;

    // Describes search node in following terms:
    // - PV_NODE: a non-terminal node with exact score
    // - CUT_NODE: a non-terminal node with lower-bound score (the score of some move exceeded beta - beta cut-off)
    // - ALL_NODE: a non-terminal node with upper-bound score (none of the moves raised alpha score)
    // - TERMINAL_NODE: a terminal node, where score is final and indicates either checkmate or stealmate
    enum Node : std::uint8_t {
        PV_NODE = 1,
        CUT_NODE,
        ALL_NODE,
        TERMINAL_NODE,

        // Derrivatives
        ROOT_NODE,
        NON_PV_NODE,

        INVALID_NODE = 0,
    };


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

    class Crawler
    {
    public:
        Crawler(TranspositionTable* tTable) 
            : virtualBoard(), evaluator(&this->virtualBoard), tTable(tTable), 
              ssTop(searchStack) {}

        // Position setup
        void setPosition(BoardConfig* board) { virtualBoard.loadPosition(*board); rootAge = virtualBoard.halfmovesPlain(); }
        void setPosition(const std::string& fen) { virtualBoard.loadPosition(fen); rootAge = virtualBoard.halfmovesPlain(); }

        // Position getters
        const BoardConfig* getPosition() const { return &virtualBoard; }

        // Main search functions
        Value search(Depth depth);                              // Initial function

        // [TESTING PURPOSES]
        // Last search data
        int non_leaf_nodes = 0;
        int leaf_nodes = 0;
        int qs_nodes = 0;

    private:
        // Search mechanisms
        template <Node stage, bool nmpAvailable>                // NMP = Null Move Pruning
        Value search(Value alpha, Value beta, Depth depth);     // Recursive subroutine
        template <Node stage>
        Value quiescence(Value alpha, Value beta, Depth depth);

        // Static evaluation
        Value evaluate() { return relative_score(evaluator.evaluate(), &virtualBoard); }

        // Search stack handlers
        void pushStack();
        void saveKiller(const Move& move);

        // Virtual board
        BoardConfig virtualBoard;

        // Evaluator
        Evaluation::Evaluator evaluator;

        // Transposition table connection
        TranspositionTable* tTable;

        // Root position details
        Age rootAge = 0;

        // Search stack implementation
        // - Search stack represents a single path in depth-first search algorithm
        // - Each node contains a relevant data for given ply
        // - ssTop is the head of the stack (represents most recently visited node)
        // - ssTop should be always incremented (pushStack method) at the beginning of search() method and decremented after return
        struct SearchInfo
        {
            int ply = -1;

            Value staticEval = NO_EVAL;
            Value eval = NO_EVAL;

            Value score = -MAX_EVAL;
            Node node = ALL_NODE;
            Move bestMove = Move::null();

            // Killer heuristic data
            Move killers[MAX_NO_KILLERS] = {};
        };

        SearchInfo searchStack[MAX_SEARCH_DEPTH + MAX_QUIESCENCE_DEPTH + 1] = {};
        SearchInfo* ssTop;
    };


    // ---------------
    // Crawler methods
    // ---------------

    inline void Crawler::pushStack()
    {
        ++ssTop;
        ssTop->ply = (ssTop - 1)->ply + 1;
        ssTop->score = -MAX_EVAL;
        ssTop->node = ALL_NODE;
        ssTop->eval = ssTop->staticEval = NO_EVAL;
        ssTop->bestMove = Move::null();
    }

    inline void Crawler::saveKiller(const Move& move)
    {
        std::rotate(ssTop->killers, ssTop->killers + MAX_NO_KILLERS - 1, ssTop->killers + MAX_NO_KILLERS);
        ssTop->killers[0] = move;
    }

}