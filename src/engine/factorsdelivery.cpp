#include "factorsdelivery.h"

namespace 
{
	FactorsVec materialBalanceFactors {
		Factor(100, 100),	// Pawn base value
		Factor(350, 350),	// Knight base value
		Factor(0, 50),		// Knight bonus for closed positions (pawn count)
		Factor(-5, -30),	// Knight penalty for distant pawns
		Factor(375, 375),	// Bishop base value
		Factor(50, 50),		// Bishop pair bonus
		Factor(0, 60),		// Bishop color weakness bonus
		Factor(580, 580),	// Rook base value
		Factor(0, -80),		// Rook penalty for global number of pawns
		Factor(1150, 1150)	// Queen base value
	};

	FactorsVec squareControlFactors {
		Factor(3, 1),		// Own camp control
		Factor(5, 3),		// Opponent`s camp control
		Factor(10, 6)		// Center control
	};

	FactorsVec pieceMobilityFactors {
		Factor(-39, 42),	// Knight mobility (opening)
		Factor(-62, 30),	// Knight mobility (endgame)
		Factor(-30, 36),	// Bishop mobility (opening)
		Factor(-55, 75),	// Bishop mobility (endgame)
		Factor(-26, 29),	// Rook mobility (opening)
		Factor(-68, 139),	// Rook mobility (endgame)
		Factor(-19, 50),	// Queen mobility (opening)
		Factor(-31, 204)	// Queen mobility (endgame)
	};

	FactorsVec pawnStructureFactors {
		Factor(-1, -9),		// Isolated pawns point 
		Factor(-10, -25),	// Doubled pawns
		Factor(-5, -12),		// Pawn islands
		Factor(-12, -32),	// Backward pawns penalty
		Factor(-21, -21),	// Backward pawns open files penalty
		Factor(25, 25),		// Passed pawns base value
		Factor(0, 32),		// Passed pawn distance multiplier
		Factor(3, 3),		// Protected passed pawn multiplier
		Factor(3, 3)		// Passed pawn point value
	};

	FactorsVec piecePlacementFactors {
		Factor(30, 18),		// Knight in common outpost bonus
		Factor(52, 36),		// Knight in uncontested outpost bonus
		Factor(76, 42),		// Knight in eternal outpost bonus
		Factor(15, 5),		// Bishop fianchetto bonus
		Factor(0, -75),		// Bad bishop penalty
		Factor(20, 20),		// Rook bonus for placement on semi-open file
		Factor(34, 34),		// Rook bonus for placement on open file
		Factor(-14, -56),	// Undeveloped rook penalty
		Factor(-256, 0),	// King distance to center penalty (opening)
		Factor(86, 0)		// King distance to center bonus (endgame)
	};

	FactorsVec connectivityFactors {
		Factor(0, 500),		// Threats points
	};

	FactorsVec kingSafetyFactors{
		Factor(0, 91),		// Pawn shield value (opening)
		Factor(0, -72),		// Semiopen files near king penalty
		Factor(28, 28),		// Max attack points for square
		Factor(0, 100)		// Attacks on square value
	};
}


namespace FactorsDelivery 
{
	FactorsMap getFactors()
	{
		return { {"MaterialBalance", materialBalanceFactors},
				 {"SquareControl", squareControlFactors},
				 {"PieceMobility", pieceMobilityFactors},
				 {"PawnStructure", pawnStructureFactors},
				 {"Connectivity", connectivityFactors},
				 {"PiecePlacement", piecePlacementFactors},
				 {"KingSafety", kingSafetyFactors} };
	}
}