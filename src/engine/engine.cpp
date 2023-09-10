#include "engine.h"
#include "pawnstructure.h"
#include "squarecontrol.h"
#include "piecemobility.h"
#include "connectivity.h"
#include "pieceplacement.h"
#include "kingsafety.h"
#include <exception>
using std::vector;
/*
/// Constructors & destructors
Engine::Engine(BoardConfig* board, const FactorsMap& factors) : realBoard(board)
{
    virtualBoard = new BoardConfig(*board);
    generator = new MoveGenerator(virtualBoard);

    virtualBoard->addObserver(generator);
    virtualBoard->connectMoveGenerator(generator);

    initElements(factors);
}

Engine::Engine(BoardConfig* board, const FactorsMap& factors, bool flag = true) : realBoard(board)
{
    virtualBoard = board;
    generator = new MoveGenerator(virtualBoard);

    virtualBoard->addObserver(generator);
    virtualBoard->connectMoveGenerator(generator);

    initElements(factors);
}

Engine::~Engine()
{
    //delete virtualBoard;
    delete generator;
}

void Engine::init()
{
    generator->generateAllMoves();
    for (PositionElement* element : elements)
        element->update();
}


/// Setting the position
void Engine::setupPosition(const std::string& FEN)
{
    if (!virtualBoard->setFromFEN(FEN))
        throw std::invalid_argument("Invalid FEN notation given to the engine\n");
    init();
}

void Engine::reloadEngine()
{
    generator->generateAllMoves();
    for (PositionElement* element : elements)
        element->update();
}



/// Evaluation
int Engine::evaluate() const
{
    int eval = 0;
    int gameStage = material->stageValueNormalised();
    for (PositionElement* element : elements)
        element->evaluate(eval, gameStage);
    eval += FactorsDelivery::sideOnMoveValue[virtualBoard->getSideOnMove()];
    return eval;
}


/// Move generation feauters
Move Engine::getRandomMove() const
{
    const MoveList* moves = generator->getLegalMoves();
    int sideID = virtualBoard->getSideOnMove() == WHITE ? 0 : 16;
    vector<Move> movesPerSide;
    for (int i = sideID; i < sideID + 16; i++)
    {
        for (const Move& move : moves[i])
            movesPerSide.push_back(move);
    }
    return movesPerSide[rand() % movesPerSide.size()];
}

bool Engine::hasLegalMoves() const
{
    const MoveList* moves = generator->getLegalMoves();
    int sideID = virtualBoard->getSideOnMove() == WHITE ? 0 : 16;
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
    int eval = evaluate();
    std::cout << "\nTotal eval: " << eval << std::endl << std::endl;

    elements[6]->show();
}
*/

namespace {
    bool checkFactors(const FactorsMap& factors, const std::string& elementName) 
    {
        return factors.find(elementName) != factors.end(); 
    }
}

Engine::Engine(const BoardConfig* board, const FactorsMap& factors, EngineBuild buildFlags)
    : realBoard(board)
{
    virtualBoard = new BoardConfig(*realBoard);

    moveGenerator = new MoveGenerator(virtualBoard);
    virtualBoard->addObserver(moveGenerator);

    initEvaluationElements(factors, buildFlags);
    updateEvaluationElements();
}

Engine::~Engine()
{
    for (PositionElement* element : evaluationElements)
        delete element;
    delete virtualBoard;
    delete moveGenerator;
}

void Engine::reloadEngine()
{
    *virtualBoard = *realBoard;
    updateEvaluationElements();
}

void Engine::initEvaluationElements(const FactorsMap& factors, const EngineBuild& buildFlags)
{
    SquareControl* control = nullptr;
    PieceMobility* mobility = nullptr;
    PawnStructure* structure = nullptr;
    Connectivity* connectivity = nullptr;
    PiecePlacement* placement = nullptr;
    KingSafety* safety = nullptr;

    if (!checkFactors(factors, "MaterialBalance"))
        throw std::logic_error("No factors provided for MaterialBalance instance\n");
    materialBalance = new MaterialBalance(virtualBoard, factors.at("MaterialBalance"));
    evaluationElements.push_back(materialBalance);
    virtualBoard->addObserver(materialBalance);

    if (!checkFactors(factors, "SquareControl"))
        throw std::logic_error("No factors provided for SquareControl instance\n");
    control = new SquareControl(moveGenerator, virtualBoard, factors.at("SquareControl"));
    evaluationElements.push_back(control);
    virtualBoard->addObserver(control);
    moveGenerator->addObserver(control);

    if (!checkFactors(factors, "PawnStructure"))
        throw std::logic_error("No factors provided for PawnStructure instance\n");
    structure = new PawnStructure(virtualBoard, materialBalance, control, factors.at("PawnStructure"));
    evaluationElements.push_back(structure);
    virtualBoard->addObserver(structure);
    materialBalance->setDependencies(structure);

    if (buildFlags.pieceMobilityFlag)
    {
        if (!checkFactors(factors, "PieceMobility"))
            throw std::logic_error("No factors provided for PieceMobility instance\n");
        mobility = new PieceMobility(virtualBoard, moveGenerator, control, factors.at("PieceMobility"));
        evaluationElements.push_back(mobility);
        moveGenerator->addObserver(mobility);
        mobility->setDependencies(materialBalance);
        control->addObserver(mobility);
    }

    if (buildFlags.connectivityFlag)
    {
        if (!checkFactors(factors, "Connectivity"))
            throw std::logic_error("No factors provided for Connectivity instance\n");
        connectivity = new Connectivity(virtualBoard, moveGenerator, control, factors.at("Connectivity"));
        evaluationElements.push_back(connectivity);
        moveGenerator->addObserver(connectivity);
    }

    if (buildFlags.piecePlacementFlag)
    {
        if (!checkFactors(factors, "PiecePlacement"))
            throw std::logic_error("No factors provided for PiecePlacement instance\n");
        placement = new PiecePlacement(virtualBoard, materialBalance, structure, control, factors.at("PiecePlacement"));
        evaluationElements.push_back(placement);
    }

    if (buildFlags.kingSafetyFlag)
    {
        if (!checkFactors(factors, "KingSafety"))
            throw std::logic_error("No factors provided for KingSafety instance\n");
        safety = new KingSafety(virtualBoard, moveGenerator, structure, factors.at("KingSafety"));
        evaluationElements.push_back(safety);
    }
}

void Engine::updateEvaluationElements()
{
    moveGenerator->generateAllMoves();
    for (PositionElement* element : evaluationElements)
        element->update();
}

int Engine::evaluate() const
{
    int eval = 0;
    int gameStage = materialBalance->stageValueNormalised();
    for (PositionElement* element : evaluationElements)
        element->evaluate(eval, gameStage);
    eval += FactorsDelivery::sideOnMoveValue[virtualBoard->getSideOnMove()];
    return eval;
}

void Engine::showPositionStats() const
{
    for (PositionElement* element : evaluationElements)
        element->show();
}