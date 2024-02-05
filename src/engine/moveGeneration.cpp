#include "moveGeneration.h"
#include <algorithm>


std::ostream& operator<<(std::ostream& os, const MoveList& moveList)
{
	for (const Move& move : moveList)
		os << move << std::endl;
	return os;
}



namespace MoveGeneration {
	template <Color side, MoveGenType gen>
	void generatePawnMoves(MoveList& moveList, const BoardConfig& board, Bitboard target)
	{
		constexpr Color enemy = ~side;
		constexpr Bitboard rank7mark = (side == WHITE ? Board::RANK_7 : Board::RANK_2);
		constexpr Bitboard rank3mark = (side == WHITE ? Board::RANK_3 : Board::RANK_6);
		constexpr Direction forwardDir = (side == WHITE ? NORTH : SOUTH);
		constexpr Direction forwardLeftDir = (side == WHITE ? NORTH_WEST : SOUTH_WEST);
		constexpr Direction forwardRightDir = (side == WHITE ? NORTH_EAST : SOUTH_EAST);

		Bitboard pawns = board.pieces(side, PAWN);
		Bitboard pawnsNotOn7 = pawns & (~rank7mark);
		Bitboard pawnsOn7 = pawns & rank7mark;
		Bitboard emptySquares = ~board.pieces();
		Bitboard enemyPieces = board.pieces(enemy);

		// TO DO: handle QUIET_CHECKS case

		// Quiet moves (single and double pushes without promotions)
		if constexpr (gen != CAPTURE) {
			Bitboard singlePushes = Bitboards::shift_s<forwardDir>(pawnsNotOn7) & emptySquares;
			Bitboard doublePushes = Bitboards::shift_s<forwardDir>(singlePushes & rank3mark) & emptySquares;
			if constexpr (gen == CHECK_EVASION) {
				singlePushes &= target;
				doublePushes &= target;
			}

			while (singlePushes) {
				Square to = Bitboards::pop_lsb(singlePushes);
				moveList.push_back(Move(to - forwardDir, to, QUIET_MOVE_FLAG));
			}
			while (doublePushes) {
				Square to = Bitboards::pop_lsb(doublePushes);
				moveList.push_back(Move(to - forwardDir - forwardDir, to, DOUBLE_PAWN_PUSH_FLAG));
			}
		}

		if constexpr (gen == CHECK_EVASION) {
			emptySquares &= target;
			enemyPieces &= target;
		}

		// Captures & enpassant
		if constexpr (gen != QUIET && gen != QUIET_CHECK) {
			// Common captures
			Bitboard leftCaptures = Bitboards::shift_s<forwardLeftDir>(pawnsNotOn7) & enemyPieces;
			Bitboard rightCaptures = Bitboards::shift_s<forwardRightDir>(pawnsNotOn7) & enemyPieces;

			while (leftCaptures) {
				Square to = Bitboards::pop_lsb(leftCaptures);
				moveList.push_back(Move(to - forwardLeftDir, to, CAPTURE_FLAG));
			}
			while (rightCaptures) {
				Square to = Bitboards::pop_lsb(rightCaptures);
				moveList.push_back(Move(to - forwardRightDir, to, CAPTURE_FLAG));
			}

			// En passant
			if (gen != CHECK_EVASION || (target & board.enpassantSquare())) {
				Bitboard enpassantPawns = pawnsNotOn7 & Board::AdjacentRankSquares[board.enpassantSquare()];
				Square to = board.enpassantSquare() + forwardDir;
				while (enpassantPawns)
					moveList.push_back(Move(Bitboards::pop_lsb(enpassantPawns), to, ENPASSANT_FLAG));
			}
		}

		// Promotions (considered as captures because of changing the general material state on the board)
		if constexpr (gen != QUIET && gen != QUIET_CHECK) {
			Bitboard quietPromotions = Bitboards::shift_s<forwardDir>(pawnsOn7)& emptySquares;
			Bitboard leftCapturePromotions = Bitboards::shift_s<forwardLeftDir>(pawnsOn7) & enemyPieces;
			Bitboard rightCapturePromotions = Bitboards::shift_s<forwardRightDir>(pawnsOn7) & enemyPieces;

			while (quietPromotions) {
				Square to = Bitboards::pop_lsb(quietPromotions);
				generatePromotions<false>(moveList, to - forwardDir, to);
			}
			while (leftCapturePromotions) {
				Square to = Bitboards::pop_lsb(leftCapturePromotions);
				generatePromotions<true>(moveList, to - forwardLeftDir, to);
			}
			while (rightCapturePromotions) {
				Square to = Bitboards::pop_lsb(rightCapturePromotions);
				generatePromotions<true>(moveList, to - forwardRightDir, to);
			}
		}
	}

	template <Color side, PieceType pieceType, MoveGenType gen>
	void generatePieceMoves(MoveList& moveList, const BoardConfig& board, Bitboard target)
	{
		constexpr Movemask flags = (gen == CAPTURE ? CAPTURE_FLAG : QUIET_MOVE_FLAG);

		Bitboard pieces = board.pieces(side, pieceType);
		while (pieces) {
			Square from = Bitboards::pop_lsb(pieces);
			Bitboard potentialMoves = Pieces::piece_attacks_s<pieceType>(from, board.pieces()) & target;
			if constexpr (gen != PSEUDO_LEGAL && gen != CHECK_EVASION) {
				while (potentialMoves)
					moveList.push_back(Move(from, Bitboards::pop_lsb(potentialMoves), flags));
			}
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

	template <Color side, MoveGenType gen>
	void generateKingMoves(MoveList& moveList, const BoardConfig& board, Bitboard target)
	{
		constexpr Movemask flags = (gen == CAPTURE ? CAPTURE_FLAG : QUIET_MOVE_FLAG);
		Square from = board.kingPosition(side);

		Bitboard potentialMoves = Pieces::piece_attacks_s<KING>(from) & target;
		if constexpr (gen != PSEUDO_LEGAL && gen != CHECK_EVASION) {
			while (potentialMoves)
				moveList.push_back(Move(from, Bitboards::pop_lsb(potentialMoves), flags));
		}
		if constexpr (gen == PSEUDO_LEGAL || gen == CHECK_EVASION) {
			Bitboard captures = potentialMoves & board.pieces();
			Bitboard quiets = potentialMoves & (~captures);
			while (quiets)
				moveList.push_back(Move(from, Bitboards::pop_lsb(quiets), QUIET_MOVE_FLAG));
			while (captures)
				moveList.push_back(Move(from, Bitboards::pop_lsb(captures), CAPTURE_FLAG));
		}

		if constexpr (gen != CAPTURE && gen != CHECK_EVASION) {
			CastleType kingside = make_castle_type(side, KINGSIDE_CASTLE);
			CastleType queenside = make_castle_type(side, QUEENSIDE_CASTLE);
			if (board.hasCastlingRight(kingside) && board.isCastlingPathClear(kingside))
				moveList.push_back(Move(from, Square(from + 2), KINGSIDE_CASTLE_FLAG));
			if (board.hasCastlingRight(queenside) && board.isCastlingPathClear(queenside))
				moveList.push_back(Move(from, Square(from - 2), QUEENSIDE_CASTLE_FLAG));
		}
	}

	template <Color side, MoveGenType gen>
	void generateSideMoves(MoveList& moveList, const BoardConfig& board)
	{
		constexpr Color enemy = ~side;

		Bitboard target = gen == QUIET ? ~board.pieces() :
						  gen == CAPTURE ? board.pieces(enemy) :
						  gen == QUIET_CHECK ? ~board.pieces() : ~board.pieces(side);	// TO DO: change after adding checks fragmentation

		if constexpr (gen == CHECK_EVASION) {
			Bitboard checkers = board.checkingPieces();

			if (Bitboards::single_populated(checkers)) {
				Square checkSquare = Bitboards::pop_lsb(checkers);
				Bitboard evasionTarget = target & (Board::Paths[board.kingPosition(side)][checkSquare] | checkSquare);
				generatePawnMoves<side, CHECK_EVASION>(moveList, board, evasionTarget);
				generatePieceMoves<side, KNIGHT, CHECK_EVASION>(moveList, board, evasionTarget);
				generatePieceMoves<side, BISHOP, CHECK_EVASION>(moveList, board, evasionTarget);
				generatePieceMoves<side, ROOK, CHECK_EVASION>(moveList, board, evasionTarget);
				generatePieceMoves<side, QUEEN, CHECK_EVASION>(moveList, board, evasionTarget);
				generateKingMoves<side, CHECK_EVASION>(moveList, board, target);
			}
			else
				generateKingMoves<side, CHECK_EVASION>(moveList, board, target);
			return;
		}

		generatePawnMoves<side, gen>(moveList, board, target);
		generatePieceMoves<side, KNIGHT, gen>(moveList, board, target);
		generatePieceMoves<side, BISHOP, gen>(moveList, board, target);
		generatePieceMoves<side, ROOK, gen>(moveList, board, target);
		generatePieceMoves<side, QUEEN, gen>(moveList, board, target);
		generateKingMoves<side, gen>(moveList, board, target);
	}

	template <MoveGenType gen>
	void generateMoves(MoveList& moveList, const BoardConfig& board)
	{
		if (board.movingSide() == WHITE)
			generateSideMoves<WHITE, gen>(moveList, board);
		else
			generateSideMoves<BLACK, gen>(moveList, board);
	}

	template void generateMoves<QUIET>(MoveList& moveList, const BoardConfig& board);
	template void generateMoves<CAPTURE>(MoveList& moveList, const BoardConfig& board);
	template void generateMoves<QUIET_CHECK>(MoveList& moveList, const BoardConfig& board);
	template void generateMoves<CHECK_EVASION>(MoveList& moveList, const BoardConfig& board);
	template void generateMoves<PSEUDO_LEGAL>(MoveList& moveList, const BoardConfig& board);

	template <>
	void generateMoves<LEGAL>(MoveList& moveList, const BoardConfig& board)
	{
		if (board.isInCheck(board.movingSide()))
			generateMoves<CHECK_EVASION>(moveList, board);
		else
			generateMoves<PSEUDO_LEGAL>(moveList, board);
		Move* legalsEnd = std::partition(moveList.begin(), moveList.end(), 
										 [&board](const Move& move) {return board.legalityCheckLight(move);});
		moveList.setEndOfList(legalsEnd);
	}
}