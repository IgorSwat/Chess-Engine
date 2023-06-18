#include "engine.h"
#include <exception>
using std::vector;

/// Constructors & destructors
Engine::Engine()
{
    config = new BoardConfig();
    generator = new MoveGenerator(config);
    analyser = new Analyser(config, generator);

    config->addObserver(generator);
    config->connectMoveGenerator(generator);
    generator->generateAllMoves();
    analyser->evaluateAll();

    externalBoard = false;
}

Engine::Engine(BoardConfig* board)
{
    config = board;
    generator = new MoveGenerator(config);
    analyser = new Analyser(config, generator);

    config->addObserver(generator);
    board->connectMoveGenerator(generator);
    generator->generateAllMoves();
    analyser->evaluateAll();

    externalBoard = true;
}

Engine::~Engine()
{
    if (!externalBoard)
        delete config;
    delete generator;
    delete analyser;
}



/// Setting the position
void Engine::setupPosition(const std::string& FEN)
{
    if (!config->setFromFEN(FEN))
        throw std::invalid_argument("Invalid FEN notation given to the engine\n");
    generator->generateAllMoves();
    analyser->evaluateAll();
}



/// Move generation feauters
Move2 Engine::getRandomMove() const
{
    const vector< vector<Move2> >& moves = generator->getLegalMoves();
    int sideID = config->getSideOnMove() == COLOR::WHITE ? 0 : 16;
    vector<Move2> movesPerSide;
    for (int i = sideID; i < sideID + 16; i++)
    {
        for (const Move2& move : moves[i])
            movesPerSide.push_back(move);
    }
    return movesPerSide[rand() % movesPerSide.size()];
}

bool Engine::hasLegalMoves() const
{
    const vector< vector<Move2> >& moves = generator->getLegalMoves();
    int sideID = config->getSideOnMove() == COLOR::WHITE ? 0 : 16;
    for (int i = sideID; i < sideID + 16; i++)
    {
        if (!moves[i].empty())
            return true;
    }
    return false;
}



/// Misc
void Engine::showPositionStats() const
{
    analyser->show();
    std::cout<<"\nGenerated moves:\n";
    //generator->showMoves(false);
    std::cout<<std::endl;
}

