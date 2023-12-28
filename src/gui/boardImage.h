#pragma once

#include "pieceImage.h"
#include "bluredCircle.h"
#include "promotionBar.h"
#include "../logic/move.h"
#include <vector>

namespace GUI {
	constexpr int MAX_PIECES_PER_TYPE = 12;
	constexpr int MAX_PIECES_NUM = 32;
}


class BoardController;
class BoardConfig;
namespace Evaluation { class Evaluator; }

struct PieceImageStack
{
	PieceImage* top() { return &pieces[topID++]; }

	int topID = 0;
	std::vector<PieceImage> pieces = {};
};



class BoardImage : public sf::Drawable
{
public:
	BoardImage(BoardController* boardController, Evaluation::Evaluator* evalutor);

	void loadPosition(const BoardConfig* board);
	void updatePieces(BoardConfig* board);
	void updatePromotion(BoardConfig* board);
	bool isWaitingForPromotion() const;

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	void clearPosition();
	void updateCheckBlur(const BoardConfig* board);
	Move translateMove(Square from, Square to, Piece piece, Piece onTargetSquare) const;
	void showEvaluationStats(const BoardConfig* board);

	BoardController* controller;
	Evaluation::Evaluator* evaluator;

	// Graphical resources & objects
	sf::VertexArray squareGrid;
	std::vector<PieceImage*> pieces;
	std::vector< PieceImageStack > piecesTables;
	sf::RectangleShape movedTo;
	sf::RectangleShape movedFrom;
	BluredCircle checkBlur;
	sf::RectangleShape promotionFog;
	PromotionBar whitePromotionBar;
	PromotionBar blackPromotionBar;
	const sf::Texture& squaresTexture;

	// Graphical logic
	PieceImage* selectedPiece = nullptr;
	PromotionBar* selectedPromotionBar = nullptr;
	Move promotionMove = NULL_MOVE;
	bool moveRectanglesState = false;
	bool bluredCircleState = false;
	bool awaitingPromotion = false;
};



inline bool BoardImage::isWaitingForPromotion() const
{
	return awaitingPromotion;
}