#pragma once

#include "positionelement.h"

class SquareControl;

class Connectivity : public PositionElement, public MoveListChangedObserver
{
private:
	BoardConfig* config;
	MoveGenerator* generator;
	SquareControl* control;

	static constexpr int MAX_ATTACK_COUNT = 4;
	static constexpr int MAX_ATTACK_POINTS = 24;
	int lastRegisteredType[8][8] { 0 };
	int lowerTypeAttackPoints[8][8][2][2] { 0 };
	int lowerTypeAttackCount[8][8][2][2] { 0 };
	int totalAttackPoints[2] { 0 };	// 0 - MAX_ATTACK_POINTS
	int totalAttackCount[2] { 0 };	// 0 - MAX_ATTACK_COUNT

	// Evaluation elements
	int threatValue[MAX_ATTACK_COUNT + 1][MAX_ATTACK_POINTS + 1] { 0 };
	
	// Helper functions
	void initTables();
	template <typename Container> void updateByRemovalConstruct(int pieceID, Container moves);

	enum AttackTypes { PAWN_ATTACK = 0, PIECE_ATTACK };
	enum Factors {COUNT_RANGE = 0};

public:
	Connectivity(BoardConfig* boardConfig, MoveGenerator* moveGenerator, SquareControl* squareControl, const FactorsVec& factors)
		: PositionElement("Connectivity", factors, 1), config(boardConfig), generator(moveGenerator), control(squareControl) {
		initTables();
	}
	void update() override;
	int evaluate(int& eval, const int& gameStage) const override;
	void updateByInsertion(const Move2& move) override;
	void updateByRemoval(int pieceID, const vector<Move2>& moves, bool legal) override { updateByRemovalConstruct(pieceID, moves); }
	void updateByRemoval(int pieceID, const MoveList& moves, bool legal) override { updateByRemovalConstruct(pieceID, moves); }
	// External functionality
	int getThreatsCount(Side side) { return totalAttackCount[side]; }
	int getThreatsPoints(Side side) { return totalAttackPoints[side]; }
	// Testing
	void show() const override;
};




template <typename Container>
void Connectivity::updateByRemovalConstruct(int pieceID, Container moves)
{
	Side side = pieceID < 16 ? WHITE : BLACK;
	int attackingPieceMappedType = mapPieceTypeByValue(config->getPiece(pieceID)->getType());
	for (const Move2& move : moves)
	{
		if (move.specialFlag.isAttacking() && attackingPieceMappedType < lastRegisteredType[move.targetPos.y][move.targetPos.x])
		{
			if (move.pieceType == PAWN && lowerTypeAttackCount[move.targetPos.y][move.targetPos.x][side][PAWN_ATTACK] > 0)
			{
				if (move.promotionFlag != PAWN && move.promotionFlag != QUEEN)
					continue;
				lowerTypeAttackCount[move.targetPos.y][move.targetPos.x][side][PAWN_ATTACK]--;
				if (lowerTypeAttackCount[move.targetPos.y][move.targetPos.x][side][PAWN_ATTACK] == 0)
				{
					lastRegisteredType[move.targetPos.y][move.targetPos.x] = 0;
					totalAttackPoints[side] -= lowerTypeAttackPoints[move.targetPos.y][move.targetPos.x][side][PAWN_ATTACK];
					if (lowerTypeAttackCount[move.targetPos.y][move.targetPos.x][side][PIECE_ATTACK] > 0)
						totalAttackPoints[side] += lowerTypeAttackPoints[move.targetPos.y][move.targetPos.x][side][PIECE_ATTACK];
					else
						totalAttackCount[side]--;
				}
			}
			else if (lowerTypeAttackCount[move.targetPos.y][move.targetPos.x][side][PIECE_ATTACK] > 0)
			{
				lowerTypeAttackCount[move.targetPos.y][move.targetPos.x][side][PIECE_ATTACK]--;
				if (lowerTypeAttackCount[move.targetPos.y][move.targetPos.x][side][PIECE_ATTACK] == 0 &&
					lowerTypeAttackCount[move.targetPos.y][move.targetPos.x][side][PAWN_ATTACK] == 0)
				{
					lastRegisteredType[move.targetPos.y][move.targetPos.x] = 0;
					totalAttackPoints[side] -= lowerTypeAttackPoints[move.targetPos.y][move.targetPos.x][side][PIECE_ATTACK];
					totalAttackCount[side]--;
				}
			}
		}
	}
}
