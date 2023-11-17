#pragma once

#include "button.h"


class Navbar : public sf::Drawable
{
public:
	Navbar(const sf::Vector2f& initialPos, const sf::Vector2f& initialSize, float buttonSize = 40.f);

	void addButton(const Button& button);
	void alignButtons(bool horizontally = true);

	ButtonType updateButtonsStates();

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	sf::Vector2f topLeftCorner;
	sf::Vector2f size;
	float buttonSize;
	float buttonSpacing;

	std::vector<Button> buttons;

	bool horizontalAlignment = true;
};



inline void Navbar::addButton(const Button& button)
{
	buttons.push_back(button);
	alignButtons(horizontalAlignment);
}

inline void Navbar::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	for (const Button& button : buttons)
		target.draw(button);
}