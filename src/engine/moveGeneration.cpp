#include "moveGeneration.h"
#include <algorithm>


// -------------
// Miscellaneous
// -------------

std::ostream& operator<<(std::ostream& os, const MoveList& moveList)
{
	for (const Move& move : moveList)
		os << move << "\n";
	return os;
}


// -----------------------
// Move generation library
// -----------------------

namespace MoveGeneration {

	template <bool isCapture>
	void generate_promotions(Square from, Square to, MoveList& moveList)
	{
		constexpr Movemask queenPromoFlags = (isCapture ? CAPTURE_FLAG | QUEEN_PROMOTION_FLAG : QUEEN_PROMOTION_FLAG);
		constexpr Movemask rookPromoFlags = (isCapture ? CAPTURE_FLAG | ROOK_PROMOTION_FLAG : ROOK_PROMOTION_FLAG);
		constexpr Movemask bishopPromoFlags = (isCapture ? CAPTURE_FLAG | BISHOP_PROMOTION_FLAG : BISHOP_PROMOTION_FLAG);
		constexpr Movemask knightPromoFlags = (isCapture ? CAPTURE_FLAG | KNIGHT_PROMOTION_FLAG : KNIGHT_PROMOTION_FLAG);

		moveList.push_back(Move(from, to, queenPromoFlags));
		moveList.push_back(Move(from, to, rookPromoFlags));
		moveList.push_back(Move(from, to, bishopPromoFlags));
		moveList.push_back(Move(from, to, knightPromoFlags));
	}

	template <MoveGenType gen, Color side>
	void generate_pawn_moves(const BoardConfig& board, Bitboard target, MoveList& moveList)
	{
		constexpr Color enemy = ~side;
		constexpr Bitboard rank7mark = (side == WHITE ? Board::RANK_7 : Board::RANK_2);
		constexpr Direction forwardDir = (side == WHITE ? NORTH : SOUTH);
		constexpr Direction forwardLeftDir = (side == WHITE ? NORTH_WEST : SOUTH_WEST);
		constexpr Direction forwardRightDir = (side == WHITE ? NORTH_EAST : SOUTH_EAST);
		Bitboard pawns = board.pieces(side, PAWN);
		Bitboard pawnsNotOn7 = pawns & (~rank7mark);
		Bitboard pawnsOn7 = pawns & rank7mark;
		Bitboard emptySquares = ~board.pieces();
		Bitboard enemyPieces = board.pieces(enemy);

		if constexpr (gen == QUIET)
			target &= ~board.possibleChecks(PAWN);
		if constexpr (gen == QUIET_CHECK)
			target &= board.possibleChecks(PAWN);

		Bitboard targetEmptySquares = target & emptySquares;
		Bitboard targetEnemyPieces = target & enemyPieces;

		// Quiet moves (single and double pushes without promotions)
		if constexpr (gen != CAPTURE) {
			constexpr Bitboard rank3mark = (side == WHITE ? Board::RANK_3 : Board::RANK_6);
			Bitboard potentiallySinglePushes = Bitboards::shift_s<forwardDir>(pawnsNotOn7) & emptySquares;
			Bitboard singlePushes = potentiallySinglePushes & targetEmptySquares;
			Bitboard doublePushes = Bitboards::shift_s<forwardDir>(potentiallySinglePushes & rank3mark) & targetEmptySquares;

			while (singlePushes) {
				Square to = Bitboards::pop_lsb(singlePushes);
				moveList.push_back(Move(to - forwardDir, to, QUIET_MOVE_FLAG));
			}
			while (doublePushes) {
				Square to = Bitboards::pop_lsb(doublePushes);
				moveList.push_back(Move(to - forwardDir - forwardDir, to, DOUBLE_PAWN_PUSH_FLAG));
			}
		}

		// Captures & enpassant
		if constexpr (gen != QUIET && gen != QUIET_CHECK) {
			// Common captures
			Bitboard leftCaptures = Bitboards::shift_s<forwardLeftDir>(pawnsNotOn7) & targetEnemyPieces;
			Bitboard rightCaptures = Bitboards::shift_s<forwardRightDir>(pawnsNotOn7) & targetEnemyPieces;
			while (leftCaptures) {
				Square to = Bitboards::pop_lsb(leftCaptures);
				moveList.push_back(Move(to - forwardLeftDir, to, CAPTURE_FLAG));
			}
			while (rightCaptures) {
				Square to = Bitboards::pop_lsb(rightCaptures);
				moveList.push_back(Move(to - forwardRightDir, to, CAPTURE_FLAG));
			}

			// EN passant
			bool isEnpassantPossible = gen != CHECK_EVASION || target & board.enpassantSquare();
			if (isEnpassantPossible) {
				Bitboard enpassantPawns = pawnsNotOn7 & Board::AdjacentRankSquares[board.enpassantSquare()];
				Square to = board.enpassantSquare() + forwardDir;
				while (enpassantPawns)
					moveList.push_back(Move(Bitboards::pop_lsb(enpassantPawns), to, ENPASSANT_FLAG));
			}

		}

		// Promotions (considered as captures because of change in general material state on the board)
		if constexpr (gen != QUIET && gen != QUIET_CHECK) {
			Bitboard quietPromotions = Bitboards::shift_s<forwardDir>(pawnsOn7) & targetEmptySquares;
			Bitboard leftCapturePromotions = Bitboards::shift_s<forwardLeftDir>(pawnsOn7) & targetEnemyPieces;
			Bitboard rightCapturePromotions = Bitboards::shift_s<forwardRightDir>(pawnsOn7) & targetEnemyPieces;

			while (quietPromotions) {
				Square to = Bitboards::pop_lsb(quietPromotions);
				generate_promotions<false>(to - forwardDir, to, moveList);
			}
			while (leftCapturePromotions) {
				Square to = Bitboards::pop_lsb(leftCapturePromotions);
				generate_promotions<true>(to - forwardLeftDir, to, moveList);
			}
			while (rightCapturePromotions) {
				Square to = Bitboards::pop_lsb(rightCapturePromotions);
				generate_promotions<true>(to - forwardRightDir, to, moveList);
			}
		}
	}


	template <MoveGenType gen, Color side, PieceType pieceType>
	void generate_piece_moves(const BoardConfig& board, Bitboard target, MoveList& moveList)
	{
		constexpr Movemask flags = (gen == CAPTURE ? CAPTURE_FLAG : QUIET_MOVE_FLAG);
		Bitboard pieces = board.pieces(side, pieceType);

		if constexpr (gen == QUIET)
			target &= ~board.possibleChecks(pieceType);
		if constexpr (gen == QUIET_CHECK)
			target &= board.possibleChecks(pieceType);

		while (pieces) {
			Square from = Bitboards::pop_lsb(pieces);
			Bitboard potentialMoves = Pieces::piece_attacks_s<pieceType>(from, board.pieces()) & target;

			// Since gen == QUIET (QUIET_CHECK) or gen == CAPTURE, all the moves are of the same type
			if constexpr (gen != PSEUDO_LEGAL && gen != CHECK_EVASION) {
				while (potentialMoves)
					moveList.push_back(Move(from, Bitboards::pop_lsb(potentialMoves), flags));
			}

			// Since we consider both captures and quiets, we can divide them for some efficiency gains
			if constexpr (gen == PSEUDO_LEGAL || gen == CHECK_EVASION) {
				Bitboard captures = potentialMoves & board.pieces();
				Bitboard quiets = potentialMoves & (~captures);
				while (quiets)
					moveList.push_back(Move(from, Bitboards::pop_lsb(quiets), QUIET_MOVE_FLAG));
				while (captures)
					moveList.push_back(Move(from, Bitboards::pop_lsb(captures), CAPTURE_FLAG));
			}
		}
	}


	// Note that we do not consider whether some square is attacked by enemy (so illegal for the king)
	// because it can be done afterwards in legality check
	template <MoveGenType gen, Color side>
	void generate_king_moves(const BoardConfig& board, Bitboard target, MoveList& moveList)
	{
		Square from = board.kingPosition(side);
		Bitboard potentialMoves = Pieces::piece_attacks_s<KING>(from) & target;

		// Since gen == QUIET (QUIET_CHECK) or gen == CAPTURE, all the moves are of the same type
		if constexpr (gen != PSEUDO_LEGAL && gen != CHECK_EVASION) {
			constexpr Movemask flags = (gen == CAPTURE ? CAPTURE_FLAG : QUIET_MOVE_FLAG);
			while (potentialMoves)
				moveList.push_back(Move(from, Bitboards::pop_lsb(potentialMoves), flags));
		}

		// Since we consider both captures and quiets, we can divide them for some efficiency gains
		if constexpr (gen == PSEUDO_LEGAL || gen == CHECK_EVASION) {
			Bitboard captures = potentialMoves & board.pieces();
			Bitboard quiets = potentialMoves & (~captures);
			while (quiets)
				moveList.push_back(Move(from, Bitboards::pop_lsb(quiets), QUIET_MOVE_FLAG));
			while (captures)
				moveList.push_back(Move(from, Bitboards::pop_lsb(captures), CAPTURE_FLAG));
		}

		// Consider castling 
		if constexpr (gen != CAPTURE && gen != CHECK_EVASION) {
			CastleType kingside = make_castle_type(side, KINGSIDE_CASTLE);
			CastleType queenside = make_castle_type(side, QUEENSIDE_CASTLE);
			if (board.hasCastlingRight(kingside) && board.isCastlingPathClear(kingside))
				moveList.push_back(Move(from, Square(from + 2), KINGSIDE_CASTLE_FLAG));
			if (board.hasCastlingRight(queenside) && board.isCastlingPathClear(queenside))
				moveList.push_back(Move(from, Square(from - 2), QUEENSIDE_CASTLE_FLAG));
		}
	}


	template <MoveGenType gen, Color side>
	void generate_side_moves(const BoardConfig& board, MoveList& moveList)
	{
		constexpr Color enemy = ~side;
		Bitboard target = gen == CAPTURE ? board.pieces(enemy) :
						  gen == QUIET_CHECK ? ~board.pieces() :
						  gen == QUIET ? ~board.pieces() :
						  ~board.pieces(side);

		// Special case - when check has to be stopped
		if constexpr (gen == CHECK_EVASION) {
			Bitboard checkers = board.checkingPieces();

			// Single check - either moving the king or blocking the check or eliminating the checker could be legal
			if (Bitboards::single_populated(checkers)) {
				Square checkSquare = Bitboards::lsb(checkers);
				Bitboard evasionTarget = target & (Board::Paths[board.kingPosition(side)][checkSquare] | checkSquare);
				generate_pawn_moves<CHECK_EVASION, side>(board, evasionTarget, moveList);
				generate_piece_moves<CHECK_EVASION, side, KNIGHT>(board, evasionTarget, moveList);
				generate_piece_moves<CHECK_EVASION, side, BISHOP>(board, evasionTarget, moveList);
				generate_piece_moves<CHECK_EVASION, side, ROOK>(board, evasionTarget, moveList);
				generate_piece_moves<CHECK_EVASION, side, QUEEN>(board, evasionTarget, moveList);
				generate_king_moves<CHECK_EVASION, side>(board, target, moveList);
			}
			// Double check - only king moves could be legal
			else
				generate_king_moves<CHECK_EVASION, side>(board, target, moveList);
		}
		// Default case - all the pseudo legal moves regardless of checks and pins
		else {
			generate_pawn_moves<gen, side>(board, target, moveList);
			generate_piece_moves<gen, side, KNIGHT>(board, target, moveList);
			generate_piece_moves<gen, side, BISHOP>(board, target, moveList);
			generate_piece_moves<gen, side, ROOK>(board, target, moveList);
			generate_piece_moves<gen, side, QUEEN>(board, target, moveList);
			if constexpr (gen != QUIET_CHECK)
				generate_king_moves<gen, side>(board, target, moveList);
		}
	}


	// -----------------
	// Global generators
	// -----------------

	template <MoveGenType gen>
	void generate_moves(const BoardConfig& board, MoveList& moveList)
	{
		if (board.movingSide() == WHITE)
			generate_side_moves<gen, WHITE>(board, moveList);
		else
			generate_side_moves<gen, BLACK>(board, moveList);
	}

	template void generate_moves<QUIET>(const BoardConfig& board, MoveList& moveList);
	template void generate_moves<CAPTURE>(const BoardConfig& board, MoveList& moveList);
	template void generate_moves<QUIET_CHECK>(const BoardConfig& board, MoveList& moveList);
	template void generate_moves<CHECK_EVASION>(const BoardConfig& board, MoveList& moveList);
	template void generate_moves<PSEUDO_LEGAL>(const BoardConfig& board, MoveList& moveList);

	// Specialization for legal moves generation (generate pseudo-legal and then check legality)
	template <>
	void generate_moves<LEGAL>(const BoardConfig& board, MoveList& moveList)
	{
		if (board.isInCheck(board.movingSide()))
			generate_moves<CHECK_EVASION>(board, moveList);
		else
			generate_moves<PSEUDO_LEGAL>(board, moveList);

		Move* legalsEnd = std::partition(moveList.begin(), moveList.end(), 
										 [&board](const Move& move) {return board.legalityCheckLight(move);});
		moveList.setEnd(legalsEnd);
	}
}