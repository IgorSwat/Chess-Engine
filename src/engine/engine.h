#pragma once

#include "materialbalance.h"
#include "factorsdelivery.h"
#include <map>

/*class Engine : public EngineObserver
{
private:
    BoardConfig* realBoard;
    BoardConfig* virtualBoard;
    MoveGenerator* generator;
    MaterialBalance* material;
    std::vector<PositionElement*> elements;
    // Helper functions
    void initElements(const FactorsMap& factors);
    bool checkFactors(const FactorsMap& factors, const std::string& elementName) { return factors.find(elementName) != factors.end(); }
    bool hasLegalMoves() const;
public:
    Engine(BoardConfig* board, const FactorsMap& factors);
    explicit Engine(BoardConfig* board, const FactorsMap& factors, bool flag);
    ~Engine();
    void init();
    void setupPosition(const std::string& FEN);
    void reloadEngine() override;
    // Evaluation
    int evaluate() const;
    // Others
    Move getRandomMove() const;
    bool isMate() const {return !hasLegalMoves() && virtualBoard->isKingChecked(virtualBoard->getSideOnMove());}
    bool isStealmate() const {return !hasLegalMoves() && !virtualBoard->isKingChecked(virtualBoard->getSideOnMove());}
    void showPositionStats() const;
};*/


struct EngineBuild
{
    bool pieceMobilityFlag;
    bool piecePlacementFlag;
    bool connectivityFlag;
    bool kingSafetyFlag;
};

class Engine : public EngineObserver
{
public:
    Engine(const BoardConfig* board, const FactorsMap& factors, EngineBuild buildFlags = {true, true, true, true});
    virtual ~Engine();
    void reloadEngine() override;
    void showPositionStats() const;

private:
    void initEvaluationElements(const FactorsMap& factors, const EngineBuild& buildFlags);
    void updateEvaluationElements();
    int evaluate() const;

    const BoardConfig* realBoard;
    BoardConfig* virtualBoard;
    MoveGenerator* moveGenerator;

    MaterialBalance* materialBalance;
    std::vector<PositionElement*> evaluationElements;
};