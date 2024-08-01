#pragma once

#include "imageRepository.h"
#include "checkMarker.h"
#include "promotionBar.h"
#include "../logic/move.h"
#include <vector>


class BoardConfig;


namespace GUI {

	// ----------------
	// BoardImage class
	// ----------------

	class BoardImage : public sf::Drawable
	{
	public:
		BoardImage(float tileSize = 100.f);

		// Position loading
		void loadPosition(const BoardConfig* board);
		void clearBoard();

		// User event processing
		// Returns (move, promotion) tuple, where move is move succesfully made by user (or null move in other case)
		// and promotion flag specifies whether move comes from promotion bar selection
		std::pair<Move, bool> update(const sf::Event& event, sf::Vector2i mousePos, const BoardConfig* board);
		// Called by BoardController after receiveing a correct promotion move
		void setupPromotionScreen(Move incompletePromotion);

		// Visualization
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	private:
		sf::Vector2f squareToCoords(Square sq) const;
		Square coordsToSquare(sf::Vector2f coords) const;

		// Graphic content - squares
		sf::VertexArray squares;				// Grid filled with square textures
		// Graphic content - pieces
		PieceImageRepository imageRepository;	// PieceImage bank
		std::vector<PieceImage*> pieces;		// Actually used piece images
		// Graphic content - move markers
		sf::RectangleShape movedFromMarker;
		sf::RectangleShape movedToMarker;
		// Graphic content - check marker
		CheckMarker checkMarker;
		// Graphic content - promotion elements
		sf::RectangleShape promotionFog;
		PromotionBar whitePromotionBar;
		PromotionBar blackPromotionBar;

		// Logic
		PieceImage* selectedPiece = nullptr;
		PromotionBar* activePromotionBar = nullptr;
		Move awaitingPromotion = Move::null();
		bool moveMarkerState = false;
		bool checkMarkerState = false;

		// Additional info
		float tileSize;

	};

}