#include "navbar.h"

namespace {
	constexpr float BUTTON_SPACING_RATIO = 1.25f;
}


Navbar::Navbar(const sf::Vector2f& initialPos, const sf::Vector2f& initialSize, float buttonSize)
	: topLeftCorner(initialPos), size(initialSize), buttonSize(buttonSize), buttonSpacing(buttonSize * BUTTON_SPACING_RATIO)
{
}

void Navbar::alignButtons(bool horizontally)
{
	int noButtons = buttons.size();
	float buttonsFieldLength = buttonSize * noButtons + buttonSpacing * (noButtons - 1);
	sf::Vector2f navbarMiddle = topLeftCorner + size * 0.5f;
	sf::Vector2f leftCorner = horizontally ? sf::Vector2f(navbarMiddle.x - buttonsFieldLength / 2.f, navbarMiddle.y - buttonSize / 2.f) :
											 sf::Vector2f(navbarMiddle.x - buttonSize / 2.f, navbarMiddle.y - buttonsFieldLength / 2.f);
	sf::Vector2f shift = horizontally ? sf::Vector2f(buttonSize + buttonSpacing, 0.f) :
										sf::Vector2f(0.f, buttonSize + buttonSpacing);
	for (Button& button : buttons) {
		button.setPosition(leftCorner);
		leftCorner += shift;
	}
	horizontalAlignment = horizontally;
}

ButtonType Navbar::updateButtonsStates()
{
	for (Button& button : buttons) {
		if (button.updateState() && !mouseLeftButtonClicked)	// If button is clicked
			return button.type();
	}
	return ButtonType::NONE;
}