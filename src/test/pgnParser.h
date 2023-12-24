#pragma once

#include "../logic/misc.h"
#include "../logic/move.h"
#include <string>
#include <sstream>

class BoardConfig;


class PGNParser
{
public:
    PGNParser(BoardConfig* board) : board(board) {}

    void loadPGN(const std::string& filepath);
    virtual bool applyNextMove();

protected:
    BoardConfig* board;
    std::istringstream input;

private:
    bool parseUntilNextMove();
    std::string parseNextMove();
    Move moveFromNotation(const std::string& moveNotation) const;
    Move castleFromNotation(const std::string& moveNotation) const;
};