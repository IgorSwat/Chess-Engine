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
		constexpr Bitboard rank7mark = (side == WHITE ? ROW_7 : ROW_2);
		constexpr Bitboard rank3mark = (side == WHITE ? ROW_3 : ROW_6);
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
			Bitboard singlePushes = Bitboards::shift<forwardDir>(pawnsNotOn7) & emptySquares;
			Bitboard doublePushes = Bitboards::shift<forwardDir>(singlePushes & rank3mark) & emptySquares;
			if constexpr (gen == CHECK_EVASION) {
				singlePushes &= target;
				doublePushes &= target;
			}

			while (singlePushes) {
				Square to = Bitboards::popLsb(singlePushes);
				moveList.push_back(Move(to - forwardDir, to, QUIET_MOVE_FLAG));
			}
			while (doublePushes) {
				Square to = Bitboards::popLsb(doublePushes);
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
			Bitboard leftCaptures = Bitboards::shift<forwardLeftDir>(pawnsNotOn7) & enemyPieces;
			Bitboard rightCaptures = Bitboards::shift<forwardRightDir>(pawnsNotOn7) & enemyPieces;

			while (leftCaptures) {
				Square to = Bitboards::popLsb(leftCaptures);
				moveList.push_back(Move(to - forwardLeftDir, to, CAPTURE_FLAG));
			}
			while (rightCaptures) {
				Square to = Bitboards::popLsb(rightCaptures);
				moveList.push_back(Move(to - forwardRightDir, to, CAPTURE_FLAG));
			}

			// En passant
			if (gen != CHECK_EVASION || (target & board.enpassantSquare())) {
				Bitboard enpassantPawns = pawnsNotOn7 & adjacentRankSquares(board.enpassantSquare());
				Square to = board.enpassantSquare() + forwardDir;
				while (enpassantPawns)
					moveList.push_back(Move(Bitboards::popLsb(enpassantPawns), to, ENPASSANT_FLAG));
			}
		}

		// Promotions (considered as captures because of changing the general material state on the board)
		if constexpr (gen != QUIET && gen != QUIET_CHECK) {
			Bitboard quietPromotions = Bitboards::shift<forwardDir>(pawnsOn7)& emptySquares;
			Bitboard leftCapturePromotions = Bitboards::shift<forwardLeftDir>(pawnsOn7) & enemyPieces;
			Bitboard rightCapturePromotions = Bitboards::shift<forwardRightDir>(pawnsOn7) & enemyPieces;

			while (quietPromotions) {
				Square to = Bitboards::popLsb(quietPromotions);
				generatePromotions<false>(moveList, to - forwardDir, to);
			}
			while (leftCapturePromotions) {
				Square to = Bitboards::popLsb(leftCapturePromotions);
				generatePromotions<true>(moveList, to - forwardLeftDir, to);
			}
			while (rightCapturePromotions) {
				Square to = Bitboards::popLsb(rightCapturePromotions);
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
			Square from = Bitboards::popLsb(pieces);
			Bitboard potentialMoves = Pieces::pieceAttacks<pieceType>(from, board.pieces()) & target;
			if constexpr (gen != PSEUDO_LEGAL && gen != CHECK_EVASION) {
				while (potentialMoves)
					moveList.push_back(Move(from, Bitboards::popLsb(potentialMoves), flags));
			}
			if constexpr (gen == PSEUDO_LEGAL || gen == CHECK_EVASION) {
				Bitboard captures = potentialMoves & board.pieces();
				Bitboard quiets = potentialMoves & (~captures);
				while (quiets)
					moveList.push_back(Move(from, Bitboards::popLsb(quiets), QUIET_MOVE_FLAG));
				while (captures)
					moveList.push_back(Move(from, Bitboards::popLsb(captures), CAPTURE_FLAG));
			}
		}
	}

	template <Color side, MoveGenType gen>
	void generateKingMoves(MoveList& moveList, const BoardConfig& board, Bitboard target)
	{
		constexpr Movemask flags = (gen == CAPTURE ? CAPTURE_FLAG : QUIET_MOVE_FLAG);
		Square from = board.kingPosition(side);

		Bitboard potentialMoves = Pieces::pieceAttacks<KING>(from) & target;
		if constexpr (gen != PSEUDO_LEGAL && gen != CHECK_EVASION) {
			while (potentialMoves)
				moveList.push_back(Move(from, Bitboards::popLsb(potentialMoves), flags));
		}
		if constexpr (gen == PSEUDO_LEGAL || gen == CHECK_EVASION) {
			Bitboard captures = potentialMoves & board.pieces();
			Bitboard quiets = potentialMoves & (~captures);
			while (quiets)
				moveList.push_back(Move(from, Bitboards::popLsb(quiets), QUIET_MOVE_FLAG));
			while (captures)
				moveList.push_back(Move(from, Bitboards::popLsb(captures), CAPTURE_FLAG));
		}

		if constexpr (gen != CAPTURE && gen != CHECK_EVASION) {
			CastleType kingside = getCastleType(side, KINGSIDE_CASTLE);
			CastleType queenside = getCastleType(side, QUEENSIDE_CASTLE);
			if (board.hasCastlingRight(kingside) && board.isCastlingPathClear(kingside))
				moveList.push_back(Move(from, from + 2, KINGSIDE_CASTLE_FLAG));
			if (board.hasCastlingRight(queenside) && board.isCastlingPathClear(queenside))
				moveList.push_back(Move(from, from - 2, QUEENSIDE_CASTLE_FLAG));
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

			if (Bitboards::isSinglePopulated(checkers)) {
				Square checkSquare = Bitboards::popLsb(checkers);
				Bitboard evasionTarget = target & (pathBetween(board.kingPosition(side), checkSquare) | checkSquare);
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
		generateMoves<PSEUDO_LEGAL>(moveList, board);
		Move* legalsEnd = std::partition(moveList.begin(), moveList.end(), 
										 [&board](const Move& move) {return board.legalityCheckLight(move);});
		moveList.setEndOfList(legalsEnd);
	}
}