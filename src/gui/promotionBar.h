#pragma once

#include "button.h"
#include <array>


namespace GUI {

	// ------------------
	// PromotionBar class
	// ------------------

	class PromotionBar : public sf::Drawable
	{
	public:
		// Helper class
		struct PromotionEntry : public InteractiveButton
		{
			sf::CircleShape circle;
			sf::Sprite icon;

			void onDefault() override;
			void onHover() override;
			void onClick() override;
		};

		PromotionBar(Color side, float tileSize = 100.f);

		// Position change
		void setPosition(sf::Vector2f pos);

		// Reacting to user events
		PieceType update(const sf::Event& event, sf::Vector2i mousePos);

		// Draw
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	private:
		void alignEntries();

		// Graphic content
		std::array<PromotionEntry, 4> entries;

		// Size and positioning
		Color side;				// Affects setPosition() by selecting the relative corner
		sf::Vector2f topLeft;
		float entrySize;

	};

}