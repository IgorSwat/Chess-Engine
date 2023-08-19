#pragma once

#include "positionelement.h"
#include "pawnstructure.h"
#include "squarecontrol.h"

enum OutpostTypes { NO_OUTPOST, COMMON_OUTPOST, UNCONTESTED_OUTPOST, ETERNAL_OUTPOST };


class PiecePlacement : public PositionElement
{
private:
	BoardConfig* config;
	MaterialBalance* material;
	PawnStructure* structure;
	SquareControl* control;

	int knightOutpostValue[33][8][4] { 0 };
	int bishopOutpostValue[33][8][4] { 0 };
	int fianchettoPointValue[33] { 0 };
	int bishopPawnPenalty[9] { 0 };
	int badBishopPawnPenalty[9] { 0 };
	int rookSemiopenFileBonus = 0;
	int rookOpenFileBonus = 0;
	int rookUndevelopedPenalty[33]{ 0 };
	int kingDistanceToCenterValue[33][7];

	bool isBishopBad(Side side, const Square& square) const;
	OutpostTypes checkForOutpost(Side side, Side opposite, const Square& square) const;
	template <PieceType type> void evaluatePiecesPlacement(int& eval, const int& gameStage) const;

	enum Factors {COMMON_OUTPOST_VALUE = 0, UNCONTESTED_OUTPOST_VALUE, ETERNAL_OUTPOST_VALUE,
		FIANCHETTO_POINT_VALUE, BAD_BISHOP_PENALTY,
		ROOK_SEMIOPEN_FILE_BONUS, ROOK_OPEN_FILE_BONUS, UNDEVELOPED_ROOK_PENALTY,
		KING_DISTANCE_TO_CENTER_OPENING, KING_DISTANCE_TO_CENTER_ENDGAME
	};
	static constexpr int factorsNum = 10;
public:
	PiecePlacement(BoardConfig* cnf, MaterialBalance* mtb, PawnStructure* strct, SquareControl* ctr, const FactorsVec& ftors) : 
		PositionElement("PiecePlacement", ftors, factorsNum), config(cnf), material(mtb), structure(strct), control(ctr) { initTables(); }
	void initTables();
	void update() override {}
	int evaluate(int& eval, const int& gameStage) const;
};