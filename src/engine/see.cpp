#include "see.h"
#include <numeric>


namespace SEE {

	constexpr int MAX_DEPTH = 33;


	// ----------------
	// Helper functions
	// ----------------

	Bitboard lvp(const BoardConfig* board, Color side, Bitboard area, PieceType &type)
	{
		for (type = PAWN; type <= KING; type = PieceType(type + 1))
		{
			Bitboard subset = area & board->pieces(side, type);
			if (subset)
				return square_to_bb(Bitboards::lsb(subset));
		}
		return 0;
	}


	// -----------
	// SEE methods
	// -----------

	int16_t evaluate(const BoardConfig* board, Square from, Square to, PieceType promotionType)
	{
		if (from == to)
			return 0;

		int16_t gain[MAX_DEPTH];
		int depth = board->movingSide();

		PieceType attackedPiece = type_of(board->onSquare(to));
		PieceType attackingPiece = type_of(board->onSquare(from));

		Bitboard occ = board->pieces();
		Bitboard fromSet = square_to_bb(from);
		Bitboard attackdef = board->attackersToSquare(to, occ);
		Bitboard mayXray = board->pieces() ^ board->pieces(KNIGHT) ^ board->pieces(KING);

		// Special case - quiet pawn move
		if (attackingPiece == PAWN && file_of(from) == file_of(to))
			attackdef |= fromSet;

		gain[depth] = PieceExchangeValue[attackedPiece] + PieceExchangeValue[promotionType] - PieceExchangeValue[PAWN];
		do {
			depth++;
			gain[depth] = PieceExchangeValue[attackingPiece] - gain[depth - 1];
			if (std::max<int16_t>(-gain[depth - 1], gain[depth]) < 0)
				break;
			attackdef ^= fromSet;
			if (fromSet & mayXray)
			{
				Bitboard xRayAttackers = (Pieces::xray_attacks<BISHOP>(to, occ, fromSet) & board->pieces(BISHOP, QUEEN)) |
										 (Pieces::xray_attacks<ROOK>(to, occ, fromSet) & board->pieces(ROOK, QUEEN));
				attackdef |= xRayAttackers;
			}
			occ ^= fromSet;
			fromSet = lvp(board, Color(depth & 0x1), attackdef, attackingPiece);
		} while (fromSet);
		while (--depth)
			gain[depth - 1] = -std::max<int16_t>(-gain[depth - 1], gain[depth]);
		return gain[board->movingSide()];
	}
	
}