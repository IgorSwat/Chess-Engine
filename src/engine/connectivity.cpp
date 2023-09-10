#include "connectivity.h"
#include "squarecontrol.h"

namespace {
	constexpr int typeWages[5]{ 0, 2, 2, 3, 6 };
	constexpr int PAWN_MULTIPLIER = 2;
	constexpr float pointsFactor = 0.67f;
}


void Connectivity::update()
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			lastRegisteredType[i][j] = 0;
			for (int k = 0; k < 2; k++)
			{
				totalAttackCount[k] = 0;
				totalAttackPoints[k] = 0;
				for (int l = 0; l < 2; l++)
				{
					lowerTypeAttackCount[i][j][k][l] = 0;
					lowerTypeAttackPoints[i][j][k][l] = 0;
				}
			}
		}
	}

	const MoveList* legal = generator->getLegalMoves();
	const MoveList* pseudoLegal = generator->getPseudoLegalMoves();
	for (int i = 0; i < 32; i++)
	{
		for (const Move& move : legal[i])
			updateByInsertion(move);
		for (const Move& move : pseudoLegal[i])
			updateByInsertion(move);
	}
}

void Connectivity::updateByInsertion(const Move& move)
{
	if (move.hasProperty(Moves::ATTACK_FLAG))
	{
		const Piece* piece = config->getPiece(move.targetPos);
		Side side = move.pieceID < 16 ? WHITE : BLACK;
		int attackedPieceMappedValue = 0;
		if (piece != nullptr && piece->getColor() != side && piece->getType() != KING &&
		 mapPieceTypeByValue(move.pieceType) < (attackedPieceMappedValue = mapPieceTypeByValue(piece->getType())))
		{
			if (move.pieceType == PAWN)
			{
				if (move.promotionType != PAWN && move.promotionType != QUEEN)
					return;
				lowerTypeAttackCount[move.targetPos.y][move.targetPos.x][side][PAWN_ATTACK]++;
				if (lowerTypeAttackCount[move.targetPos.y][move.targetPos.x][side][PAWN_ATTACK] == 1)
				{
					lastRegisteredType[move.targetPos.y][move.targetPos.x] = attackedPieceMappedValue;
					lowerTypeAttackPoints[move.targetPos.y][move.targetPos.x][side][PAWN_ATTACK] = PAWN_MULTIPLIER * typeWages[piece->getType()];
					totalAttackPoints[side] += lowerTypeAttackPoints[move.targetPos.y][move.targetPos.x][side][PAWN_ATTACK];
					if (lowerTypeAttackCount[move.targetPos.y][move.targetPos.x][side][PIECE_ATTACK] > 0)
						totalAttackPoints[side] -= lowerTypeAttackPoints[move.targetPos.y][move.targetPos.x][side][PIECE_ATTACK];
					else
						totalAttackCount[side]++;
				}
			}
			else
			{
				lowerTypeAttackCount[move.targetPos.y][move.targetPos.x][side][PIECE_ATTACK]++;
				if (lowerTypeAttackCount[move.targetPos.y][move.targetPos.x][side][PIECE_ATTACK] == 1)
				{
					lastRegisteredType[move.targetPos.y][move.targetPos.x] = attackedPieceMappedValue;
					lowerTypeAttackPoints[move.targetPos.y][move.targetPos.x][side][PIECE_ATTACK] = typeWages[piece->getType()];
					if (lowerTypeAttackCount[move.targetPos.y][move.targetPos.x][side][PAWN_ATTACK] == 0)
					{
						totalAttackPoints[side] += lowerTypeAttackPoints[move.targetPos.y][move.targetPos.x][side][PIECE_ATTACK];
						totalAttackCount[side]++;
					}
				}
			}
			totalAttackCount[side] = std::min(MAX_ATTACK_COUNT, totalAttackCount[side]);
			totalAttackPoints[side] = std::min(MAX_ATTACK_POINTS, totalAttackPoints[side]);
		}
	}
}

void Connectivity::show() const
{
	PositionElement::show();
	std::cout << "Threats number in white`s favour: " << totalAttackCount[0] << std::endl;
	std::cout<< "Threats number in black`s favour: " << totalAttackCount[1] << std::endl;
	std::cout << "Threats points in white`s favour: " << totalAttackPoints[0] << std::endl;
	std::cout << "Threats points in black`s favour: " << totalAttackPoints[1] << std::endl;
}



/// Evaluation
void Connectivity::initTables()
{
	for (int i = 0; i < MAX_ATTACK_COUNT; i++)
	{
		int upperBound = interpolation(factors[COUNT_RANGE], i / static_cast<float>(MAX_ATTACK_COUNT), [](float x)->float {return x * x; });
		int lowerBound = static_cast<int>(upperBound * pointsFactor);
		for (int j = 0; j < MAX_ATTACK_POINTS; j++)
			threatValue[i][j] = interpolation(lowerBound, upperBound, j / static_cast<float>(MAX_ATTACK_POINTS), [](float x)->float {return std::sqrt(x); });
	}
}

int Connectivity::evaluate(int& eval, const int& gameStage) const
{
	int sum = 0;
	sum += threatValue[totalAttackCount[0]][totalAttackPoints[0]];
	sum -= threatValue[totalAttackCount[1]][totalAttackPoints[1]];
	eval += sum;
	std::cout << "Connectivity evaluation: " << sum << std::endl;
	return sum;
}
