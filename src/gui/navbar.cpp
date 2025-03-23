#include "navbar.h"


namespace GUI {

	// Customizable parameters
	constexpr float BUTTON_SPACING_RATIO = 1.25f;


	// --------------
	// Navbar methods
	// --------------

	Navbar::Navbar(sf::Vector2f pos, sf::Vector2f size, float buttonSize)
		: topLeft(pos), size(size), buttonSize(buttonSize), buttonSpacing(buttonSize * BUTTON_SPACING_RATIO)
	{
	}

	void Navbar::addButton(const RoundedButton& button)
	{
		buttons.push_back(button);
		alignButtons();
	}

	ButtonType Navbar::update(const sf::Event& event, sf::Vector2i mousePos)
	{
		ButtonType result = ButtonType::NONE;

		// Navbar accepts only mouse left clicks
		if (event.type == sf::Event::MouseButtonPressed && int(event.key.code) != int(sf::Mouse::Left))
			return result;

		for (RoundedButton& button : buttons) {
			if (button.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
				if (button.update(event.type, true))
					result = button.type();
			}
			else
				button.update(sf::Event::MouseMoved, false);
		}

		return result;
	}

	void Navbar::draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		for (const RoundedButton& button : buttons)
			target.draw(button, states);
	}

	void Navbar::alignButtons(bool horizontally)
	{
		float buttonFieldLength = buttonSize * buttons.size() + buttonSpacing * (buttons.size() - 1);

		sf::Vector2f middle = topLeft + size * 0.5f;
		sf::Vector2f leftCorner = horizontally ? sf::Vector2f(middle.x - buttonFieldLength / 2.f, middle.y - buttonSize / 2.f) :
															sf::Vector2f(middle.x - buttonSize / 2.f, middle.y - buttonFieldLength / 2.f);
		sf::Vector2f shift = horizontally ? sf::Vector2f(buttonSize + buttonSpacing, 0.f) :
											sf::Vector2f(0.f, buttonSize + buttonSpacing);
		
		for (RoundedButton& button : buttons) {
			button.setPosition(leftCorner);
			leftCorner += shift;
		}
	}

}