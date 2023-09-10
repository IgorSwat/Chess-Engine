#pragma once

#include "positionelement.h"
#include "pawnstructure.h"

class KingSafety : public PositionElement, public PositionChangedObserver, public MoveListChangedObserver
{
public:
	static constexpr int factorsNum = 4;
	KingSafety(BoardConfig* cnf, MoveGenerator* gen, PawnStructure* strct, const FactorsVec& factors);
	~KingSafety() { delete attacksOnSquareValue; }

	void update() override;
	void updateByMove(int pieceID, const Square& oldPos, const Square& newPos) override;
	void updateByInsertion(const Move& move) override;
	void updateByRemoval(int pieceID, const vector<Move>& moves, bool legal) override;
	void updateByRemoval(int pieceID, const MoveList& moves, bool legal) override;

	int evaluate(int& eval, const int& gameStage) const override;
	void show() const override;
private:
	enum Factors {PAWN_SHIELD_VALUE = 0, SEMIOPEN_FILES_NEAR_KING_PENALTY,
				  MAX_ATTACK_POINTS_FOR_SQUARE, ATTACK_POINTS_VALUE_FOR_SQUARE};

	bool isShieldingKing(Side side, const Square& pawnPos) const;
	void countShieldingPawns(Side side, int& counter, const std::vector<int>& pawnRows, int kingRow);
	void calculatePawnsShield(Side side);
	int calculateSemiopenFilesNearKing(Side side, Side opposite) const;
	bool isInKingRange(const Square& square, const Square& kingPos) const;
	void calculateAttacksNearKing(Side side, Side opposite);
	void initEvaluationTables();
	void evaluatePawnShield(int& eval, const int& gameStage) const;
	void evaluateSemiopenFilesNearKing(int& eval, const int& gameStage) const;
	void evaluateAttacksNearKing(int& eval, const int& gameStage) const;
	template <typename Container> void updateByRemovalConstruct(int pieceID, const Container& moves, bool legal);

	BoardConfig* config;
	MoveGenerator* generator;
	PawnStructure* structure;

	int pawnShieldCount[2] { 0 };
	int attacksTable[8][8][2] { 0 };
	int attacksNearKingEval[2] { 0 };
	static constexpr int pieceAttacksValues[6] { 2, 4, 5, 7, 10, 0 };

	int pawnShieldValue[33][9] { 0 };
	int semiopenFilesNearKingPenalty[33][4] { 0 };
	int maxAttackPointsForSquare = 0;
	int* attacksOnSquareValue;
};



template <typename Container>
void KingSafety::updateByRemovalConstruct(int pieceID, const Container& moves, bool legal)
{
	Side side = pieceID < 16 ? WHITE : BLACK;
	for (const Move& move : moves)
	{
		if (isInKingRange(move.targetPos, config->getKing(opposition[side])->getPosition()))
		{
			attacksNearKingEval[side] -= attacksOnSquareValue[std::min(maxAttackPointsForSquare, attacksTable[move.targetPos.y][move.targetPos.x][side])];
			attacksTable[move.targetPos.y][move.targetPos.x][side] -= pieceAttacksValues[move.pieceType];
			attacksNearKingEval[side] += attacksOnSquareValue[std::min(maxAttackPointsForSquare, attacksTable[move.targetPos.y][move.targetPos.x][side])];
		}
	}
}