#include "see.h"


namespace {

	constexpr int PieceExchangeValue[PIECE_TYPE_RANGE] = { 0, 100, 325, 325, 500, 900, 100000, 0 };

}



int SEE::evaluate(Square from, Square to)
{
	// Check whether a real exchange happens (Possible to also check the color of both pieces)
	if (board->isFree(from) || board->isFree(to))
		return 0;

	int depth = board->movingSide();
	Bitboard mayXray = board->pieces() ^ board->pieces(KNIGHT) ^ board->pieces(KING);
	Bitboard occ = board->pieces();
	Bitboard fromSet = square_to_bb(from);
	Bitboard attackdef = board->attackersToSquare(to, occ);
	PieceType attackedPiece = type_of(board->onSquare(to));
	PieceType attackingPiece = type_of(board->onSquare(from));

	gain[depth] = PieceExchangeValue[attackedPiece];
	do {
		depth++;
		gain[depth] = PieceExchangeValue[attackingPiece] - gain[depth - 1];
		if (std::max(-gain[depth - 1], gain[depth]) < 0)
			break;
		attackdef ^= fromSet;
		if (fromSet & mayXray) {
			Bitboard xRayAttackers = (Pieces::xray_attacks<BISHOP>(to, occ, fromSet) & board->pieces(BISHOP, QUEEN)) |
				(Pieces::xray_attacks<ROOK>(to, occ, fromSet) & board->pieces(ROOK, QUEEN));
			attackdef |= xRayAttackers;
		}
		occ ^= fromSet;
		fromSet = lvp(Color(depth & 0x1), attackdef, attackingPiece);
	} while (fromSet);
	while (--depth)
		gain[depth - 1] = -std::max(-gain[depth - 1], gain[depth]);
	return gain[board->movingSide()];
}

Bitboard SEE::lvp(Color side, Bitboard area, PieceType& type) const
{
	for (type = PAWN; type <= KING; type = PieceType(type + 1)) {
		Bitboard subset = area & board->pieces(side, type);
		if (subset)
			return square_to_bb(Bitboards::lsb(subset));
	}
	return 0;
}