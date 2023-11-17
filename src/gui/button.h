#pragma once

#include "roundedRectangle.h"
#include "textures.h"

enum class ButtonType {
	BACK = 0, RESET,
	NONE
};


class Button : public sf::Drawable
{
public:
	Button(ButtonType btype, float size, const sf::Texture& contentTexture);

	void setPosition(const sf::Vector2f& pos);
	void setPosition(float xPos, float yPos);

	bool updateState();	// Updates button and returns true if button is clicked, and false otherwise
	ButtonType type() const;

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	void centraliseContent();
	void scaleContent(const sf::Texture& contentTexture);

	// Graphical objects
	RoundedRectangle background;
	sf::Sprite content;

	// Logic
	ButtonType buttonType;
	bool isMouseAbove = false;
	bool isClicked = false;
};



inline void Button::setPosition(float xPos, float yPos) 
{ 
	setPosition(sf::Vector2f(xPos, yPos)); 
}

inline ButtonType Button::type() const
{
	return buttonType;
}

inline void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(background, states);
	target.draw(content, states);
}