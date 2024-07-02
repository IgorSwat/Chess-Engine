#pragma once

#include "utilities.h"
#include "../logic/boardConfig.h"


// ---------------------
// PGN parser main class
// ---------------------

class PgnParser
{
public:
    PgnParser(const std::string& pgnFilePath, BoardConfig* board);
    virtual ~PgnParser() {}

    virtual bool processNext();     // Can be overriden by derived classes to add new functionalities using PGN notation
    void processAll();
    virtual void reset();

protected:
    bool parseToNextMove();                                        // Returns true if next move was succesfully found
    std::string parseMove();                                       // Parses next move notation
    Move moveFromNotation(const std::string& notation) const;      // Creates a move from notation

    // Helper parsing functions
    static Square squareFromNotation(const std::string& notation);

    std::string pgn;
    std::string::iterator currentPos;
    BoardConfig* board;

    // For parsing inbetween spaces
    int bracketCount = 0;
};