#pragma once

#include "button.h"
#include <vector>


namespace GUI {

	// ------------
	// Navbar class
	// ------------

	class Navbar : public sf::Drawable
	{
	public:
		Navbar(sf::Vector2f pos, sf::Vector2f size, float buttonSize = 40.f);

		// Manipulating button list
		void addButton(const RoundedButton& button);

		// Main state update, returns the clicked button (if any)
		ButtonType update(const sf::Event& event, sf::Vector2i mousePos = sf::Vector2i(0, 0));

		// Draw
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	private:
		void alignButtons(bool horizontally = true);

		// Graphic content
		std::vector<RoundedButton> buttons;

		// Size and alignment
		sf::Vector2f topLeft;
		sf::Vector2f size;
		float buttonSize;
		float buttonSpacing;

	};

}