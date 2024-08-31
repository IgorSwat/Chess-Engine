#pragma once

#include "moveSelection.h"
#include "evaluation.h"


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
        PV_NODE = 1,    // Exact value
        CUT_NODE,       // A lower-bound score (a line which should not be allowed by enemy)
        ALL_NODE,       // An upper-bound score (no move exceeds alpha - best possible score for moving side)
        TERMINAL_NODE,  // A node where checkmate or stealmate appears

        INVALID_NODE = 0,
        NODE_TYPE_RANGE = 3
    };

    // Specifies behavior of search subroutine
    enum SearchStage : std::uint8_t {
        // Main search stages
        ROOT_STAGE_I = 1,   // At initial, root position
        ROOT_STAGE_II,      // Exactly one play after root
        COMMON_STAGE,

        // Quiescence search stages
        Q_ROOT_STAGE,
        Q_COMMON_STAGE,

        INVALID_STAGE = 0,
        SEARCH_STAGE_RANGE = 5
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

    // Main search mechanism
    class Crawler
    {
    public:
        Crawler(TranspositionTable* tTable) : virtualBoard(), evaluator(&this->virtualBoard), tTable(tTable) {}

        // Setup
        void setPosition(BoardConfig* board);
        void setPosition(const std::string& fen);

        // Main search functions
        Value search(Depth depth);                              // Initial function

    private:
        Value evaluate();                                       // Stand pat evaluation = search on depth 0
        template <SearchStage stage>
        Value search(Value alpha, Value beta, Depth depth);     // Recursive subroutine
        template <SearchStage stage>
        Value quiescence(Value alpha, Value beta, Depth depth);

        // Virtual board and related properties
        BoardConfig virtualBoard;
        Age rootAge = 0;

        // Additional search info
        Depth lastUsedDepth = 0;

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

}