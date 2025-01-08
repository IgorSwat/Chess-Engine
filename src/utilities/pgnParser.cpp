#include "pgnParser.h"
#include <cctype>
#include <algorithm>


// ----------------------------
// Helper functions and defines
// ----------------------------

namespace {

    constexpr bool isOpeningBracket(char c) {return c == '(' || c == '[' || c == '{';}
    constexpr bool isClosingBracket(char c) {return c == ')' || c == ']' || c == '}';}
    constexpr bool isNotationEnd(char c) {return c == ' ' || c == '#' || c == '+' || c == '!' || c == '?';}

}


// -----------------
// PgnParser methods
// -----------------

namespace Utilities::Parsing {

    PgnParser::PgnParser(const std::string &pgnFilePath, BoardConfig *board)
        : pgn(Utilities::read_file(pgnFilePath)), currentPos(pgn.begin()), board(board)
    {
    }

    bool PgnParser::processNext()
    {
        if (!parseToNextMove())
            return false;
        std::string notation = parseMove();

        Move move = Utilities::Parsing::parse_move(*board, notation);
        if (move == Move::null())
            return false;

        board->makeMove(move);
        return true;
    }

    void PgnParser::processAll()
    {
        while (processNext())
        {
        }
    }

    void PgnParser::reset()
    {
        currentPos = pgn.begin();
        bracketCount = 0;
    }

    bool PgnParser::parseToNextMove()
    {
        while (currentPos != pgn.end())
        {
            char c = *currentPos;
            if (isalpha(c) && bracketCount == 0)
                return true;
            if (isOpeningBracket(c))
                bracketCount++;
            else if (isClosingBracket(c))
                bracketCount--;
            currentPos++;
        }
        return false;
    }

    std::string PgnParser::parseMove()
    {
        std::string::iterator notationEnd = std::find_if(currentPos, pgn.end(), isNotationEnd);
        std::string result = notationEnd != pgn.end() ? pgn.substr(std::distance(pgn.begin(), currentPos), std::distance(currentPos, notationEnd)) : "";
        currentPos = notationEnd;
        return result;
    }
}