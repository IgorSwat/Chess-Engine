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

    enum NodeType : std::uint8_t {
        PV_NODE = 1,    // Exact value
        CUT_NODE,
        ALL_NODE,

        INVALID_NODE = 0,
        NODE_TYPE_RANGE = 3
    };


    // ----------------
    // Helper functions
    // ----------------

    inline Age distance(Age rootAge, Age age)
    {
        return age > rootAge ? age - rootAge : rootAge - age;
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
        Value search(Value alpha, Value beta, Depth depth);     // Recursive subfunction

        // Evaluation functions
        Value relativeEval(Value eval) const;   // For NegaMax algorithm purposes
        Value evaluate();                       // Stand pat evaluation = search on depth 0

    private:
        // Virtual board and related properties
        BoardConfig virtualBoard;
        Age rootAge = 0;

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

    inline Value Crawler::relativeEval(Value eval) const
    {
        return virtualBoard.movingSide() == WHITE ? eval : -eval;
    }

    inline Value Crawler::evaluate()
    {
        return relativeEval(evaluator.evaluate());
    }

}