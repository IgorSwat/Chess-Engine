#pragma once

#include "parsing.h"
#include "../logic/boardConfig.h"


// ---------------------
// PGN parser main class
// ---------------------

namespace Utilities::Parsing {

    class PgnParser
    {
    public:
        PgnParser(const std::string &pgnFilePath, BoardConfig *board);
        virtual ~PgnParser() {}

        virtual bool processNext(); // Can be overriden by derived classes to add new functionalities using PGN notation
        void processAll();
        virtual void reset();

    protected:
        // Helper functions
        bool parseToNextMove();  // Returns true if next move was succesfully found
        std::string parseMove(); // Parses next move notation

        // Board connection
        BoardConfig *board;

        // Parsing logic
        std::string pgn;
        std::string::iterator currentPos;
        int bracketCount = 0;
    };
    
}