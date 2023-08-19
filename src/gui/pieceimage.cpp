#include "pieceimage.h"

PieceImage::PieceImage(const sf::Vector2f& initPos)
	: stablePosition(initPos), state(false)
{
	image.setPosition(stablePosition);
}

PieceImage::PieceImage(const sf::Texture& texture, const sf::Vector2f& initPos)
	: stablePosition(initPos), image(texture), state(true)
{
	image.setPosition(stablePosition);
}



void PieceImage::setTexture(const sf::Texture& texture)
{
	image.setTexture(texture);
}

void PieceImage::setState(bool newState)
{
	state = newState;
}

void PieceImage::setTempPosition(const sf::Vector2f& pos)
{
	image.setPosition(pos);
}

void PieceImage::setPermPosition(const sf::Vector2f& pos)
{
	stablePosition = pos;
	resetToStablePosition();
}

void PieceImage::resetToStablePosition()
{
	image.setOrigin({ 0.f, 0.f });
	image.setPosition(stablePosition);
}

void PieceImage::updateOrigin(const sf::Vector2f& mousePos)
{
	image.setOrigin({ mousePos.x - stablePosition.x, mousePos.y - stablePosition.y });
}

void PieceImage::updateOrigin(const sf::Vector2i& mousePos)
{
	updateOrigin(sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)));
}

void PieceImage::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (isActive())
		target.draw(image);
}