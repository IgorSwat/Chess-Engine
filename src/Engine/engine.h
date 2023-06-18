#ifndef ENGINE_H
#define ENGINE_H

#include "analyser.h"

class Engine
{
private:
    BoardConfig* config;
    MoveGenerator* generator;
    Analyser* analyser;
    bool externalBoard;
    /// Helper functions
    bool hasLegalMoves() const;
public:
    Engine();
    Engine(BoardConfig* board);
    ~Engine();
    void setupPosition(const std::string& FEN);
    Move2 getRandomMove() const;
    bool isMate() const {return !hasLegalMoves() && config->isKingChecked(config->getSideOnMove());}
    bool isStealmate() const {return !hasLegalMoves() && !config->isKingChecked(config->getSideOnMove());}
    void showPositionStats() const;
};

#endif // ENGINE_H
