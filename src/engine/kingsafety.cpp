#include "kingsafety.h"
#include <functional>

constexpr int KingSafety::pieceAttacksValues[];

namespace {
	std::function<bool(int, int)> isInFrontOfKing[2]{
		[](int pawnRow, int kingRow)->bool {return pawnRow >= kingRow - 2 && pawnRow <= kingRow; },
		[](int pawnRow, int kingRow)->bool {return pawnRow <= kingRow + 2 && pawnRow >= kingRow; }
	};
}


KingSafety::KingSafety(BoardConfig* cnf, MoveGenerator* gen, PawnStructure* strct, const FactorsVec& factors)
	: PositionElement("KingSafety", factors, factorsNum), config(cnf), generator(gen), structure(strct)
{
	initEvaluationTables();
}

void KingSafety::update()
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			attacksTable[i][j][WHITE] = 0;
			attacksTable[i][j][BLACK] = 0;
		}
	}
	attacksNearKingEval[WHITE] = attacksNearKingEval[BLACK] = 0;
	const MoveList* legal = generator->getLegalMoves();
	const MoveList* pseudoLegal = generator->getPseudoLegalMoves();
	for (int i = 0; i < 32; i++)
	{
		for (const Move& move : legal[i])
			updateByInsertion(move);
		for (const Move& move : pseudoLegal[i])
			updateByInsertion(move);
	}
	calculatePawnsShield(WHITE);
	calculatePawnsShield(BLACK);
}

void KingSafety::updateByMove(int pieceID, const Square& oldPos, const Square& newPos)
{
	const Piece* piece = config->getPiece(pieceID);
	Side side = piece->getColor();
	switch (piece->getType())
	{
	case PAWN:
		if (oldPos.x < 8 && isShieldingKing(side, oldPos))
			pawnShieldCount[side] -= 1;
		if (newPos.x < 8 && isShieldingKing(side, newPos))
			pawnShieldCount[side] += 1;
		break;
	case KING:
		pawnShieldCount[side] = 0;
		calculatePawnsShield(side);
		break;
	}
}

void KingSafety::updateByInsertion(const Move& move)
{
	Side side = move.pieceID < 16 ? WHITE : BLACK;
	if (isInKingRange(move.targetPos, config->getKing(opposition[side])->getPosition()))
	{
		attacksNearKingEval[side] -= attacksOnSquareValue[std::min(maxAttackPointsForSquare, attacksTable[move.targetPos.y][move.targetPos.x][side])];
		attacksTable[move.targetPos.y][move.targetPos.x][side] += pieceAttacksValues[move.pieceType];
		attacksNearKingEval[side] += attacksOnSquareValue[std::min(maxAttackPointsForSquare, attacksTable[move.targetPos.y][move.targetPos.x][side])];
	}
}

void KingSafety::updateByRemoval(int pieceID, const vector<Move>& moves, bool legal)
{
	updateByRemovalConstruct<vector<Move>>(pieceID, moves, legal);
}

void KingSafety::updateByRemoval(int pieceID, const MoveList& moves, bool legal)
{
	updateByRemovalConstruct<MoveList>(pieceID, moves, legal);
}

void KingSafety::show() const
{
	std::cout << "Pawn shield (white | black): " << pawnShieldCount[WHITE] << " | " << pawnShieldCount[BLACK] << std::endl;
}



// Helper functions
bool KingSafety::isShieldingKing(Side side, const Square& pawnPos) const
{
	const Square& kingPos = config->getKing(side)->getPosition();
	return pawnPos.x >= kingPos.x - 1 && pawnPos.x <= kingPos.x + 1 && isInFrontOfKing[side](pawnPos.y, kingPos.y);
}

void KingSafety::countShieldingPawns(Side side, int& counter, const std::vector<int>& pawnRows, int kingRow)
{
	for (const int& pawnRow : pawnRows)
	{
		if (isInFrontOfKing[side](pawnRow, kingRow))
			counter += 1;
	}
}

void KingSafety::calculatePawnsShield(Side side)
{
	int counter = 0;
	const Square& kingPos = config->getKing(side)->getPosition();
	countShieldingPawns(side, counter, structure->getPawnRowsAtColumn(side, kingPos.x), kingPos.y);
	if (kingPos.x != 0)
		countShieldingPawns(side, counter, structure->getPawnRowsAtColumn(side, kingPos.x - 1), kingPos.y);
	if (kingPos.x != 7)
		countShieldingPawns(side, counter, structure->getPawnRowsAtColumn(side, kingPos.x + 1), kingPos.y);
	pawnShieldCount[side] = counter;
}

int KingSafety::calculateSemiopenFilesNearKing(Side side, Side opposite) const
{
	int openFilesCount = 0;
	const Square& kingPos = config->getKing(side)->getPosition();
	openFilesCount += static_cast<int>(kingPos.x != 0 && structure->isFileSemiOpen(opposite, kingPos.x - 1));
	openFilesCount += static_cast<int>(structure->isFileSemiOpen(opposite, kingPos.x));
	openFilesCount += static_cast<int>(kingPos.x != 7 && structure->isFileSemiOpen(opposite, kingPos.x + 1));
	return openFilesCount;
}

bool KingSafety::isInKingRange(const Square& square, const Square& kingPos) const
{
	return square.x >= kingPos.x - 1 && square.x <= kingPos.x + 1 && square.y >= kingPos.y - 1 && square.y <= kingPos.y + 1;
}

void KingSafety::calculateAttacksNearKing(Side side, Side opposite)
{
	attacksNearKingEval[side] = 0;
	const Square& kingPos = config->getKing(side)->getPosition();
	for (int i = std::max(0, kingPos.y - 1); i <= std::min(7, kingPos.y + 1); i++)
	{
		for (int j = std::max(0, kingPos.x - 1); j <= std::min(7, kingPos.x + 1); j++)
			attacksNearKingEval[side] += attacksOnSquareValue[std::min(maxAttackPointsForSquare, attacksTable[i][j][opposite])];
	}
}



// Evaluation
void KingSafety::initEvaluationTables()
{
	maxAttackPointsForSquare = factors[MAX_ATTACK_POINTS_FOR_SQUARE].startValue;
	attacksOnSquareValue = new int[maxAttackPointsForSquare + 1];
	for (int i = 0; i < 33; i++)
	{
		float w = (32.f - i) / 32.f;
		int pawnShieldStageInterpolation = interpolation(factors[PAWN_SHIELD_VALUE].endValue, 0, w, [](float x)->float {return x; });
		for (int j = 0; j < 9; j++)
			pawnShieldValue[i][j] = interpolation(0, pawnShieldStageInterpolation, j / 8.f, [](float x)->float {return std::sqrt(x); });
		int semiopenFilesNearKingStageInterpolation = interpolation(factors[SEMIOPEN_FILES_NEAR_KING_PENALTY].endValue, 0, w, [](float x)->float {return x; });
		for (int j = 0; j < 4; j++)
			semiopenFilesNearKingPenalty[i][j] = interpolation(0, semiopenFilesNearKingStageInterpolation, j / 3.f, [](float x)->float {return x; });
	}
	for (int i = 0; i < maxAttackPointsForSquare + 1; i++)
		attacksOnSquareValue[i] = interpolation(factors[ATTACK_POINTS_VALUE_FOR_SQUARE], i / static_cast<float>(maxAttackPointsForSquare), [](float x)->float {return x * x; });
}

void KingSafety::evaluatePawnShield(int& eval, const int& gameStage) const
{
	int startValue = eval;
	eval += pawnShieldValue[gameStage][pawnShieldCount[WHITE]];
	eval -= pawnShieldValue[gameStage][pawnShieldCount[BLACK]];
	std::cout << "Pawn shield evaluation: " << eval - startValue << std::endl;
}

void KingSafety::evaluateSemiopenFilesNearKing(int& eval, const int& gameStage) const
{
	int startValue = eval;
	eval += semiopenFilesNearKingPenalty[gameStage][calculateSemiopenFilesNearKing(WHITE, BLACK)];
	eval -= semiopenFilesNearKingPenalty[gameStage][calculateSemiopenFilesNearKing(BLACK, WHITE)];
	std::cout << "Open files near king evaluation: " << eval - startValue << std::endl;
}

void KingSafety::evaluateAttacksNearKing(int& eval, const int& gameStage) const
{
	int startValue = eval;
	eval += attacksNearKingEval[WHITE];
	eval -= attacksNearKingEval[BLACK];
	std::cout << "Attacks near king evaluation: " << eval - startValue << std::endl;
}

int KingSafety::evaluate(int& eval, const int& gameStage) const
{
	evaluatePawnShield(eval, gameStage);
	evaluateSemiopenFilesNearKing(eval, gameStage);
	evaluateAttacksNearKing(eval, gameStage);
	return 0;
}
