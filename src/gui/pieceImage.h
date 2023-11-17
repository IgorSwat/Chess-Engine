#pragma once

#include "../logic/misc.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

class PieceImage : public sf::Drawable
{
public:
	PieceImage(Piece piece, const sf::Texture& texture)
		: representedPiece(piece), image(texture), stablePosition({ 0.f, 0.f }) {
		image.setPosition(stablePosition);
	}

	PieceImage(Piece piece, const sf::Texture& texture, const sf::Vector2f& initialPos)	
		: representedPiece(piece), image(texture), stablePosition(initialPos) {
		image.setPosition(stablePosition);
	}

	// Position manipulations
	void setTempPosition(const sf::Vector2f& pos);
	void setPermPosition(const sf::Vector2f& pos);
	void resetToStablePosition();
	void updateOrigin(const sf::Vector2f& mousePos);
	void updateOrigin(const sf::Vector2i& mousePos);

	const sf::Vector2f& getStablePosition() const;
	const sf::Vector2f& getCurrentPosition() const;
	sf::FloatRect getGlobalBounds() const;
	Piece piece() const;

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	Piece representedPiece;
	sf::Sprite image;
	sf::Vector2f stablePosition;
};



inline void PieceImage::setTempPosition(const sf::Vector2f& pos)
{
	image.setPosition(pos);
}

inline void PieceImage::setPermPosition(const sf::Vector2f& pos)
{
	stablePosition = pos;
	resetToStablePosition();
}

inline void PieceImage::resetToStablePosition()
{
	image.setOrigin({ 0.f, 0.f });
	image.setPosition(stablePosition);
}

inline void PieceImage::updateOrigin(const sf::Vector2f& mousePos)
{
	image.setOrigin({ mousePos.x - stablePosition.x, mousePos.y - stablePosition.y });
}

inline void PieceImage::updateOrigin(const sf::Vector2i& mousePos)
{
	updateOrigin(sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)));
}

inline const sf::Vector2f& PieceImage::getStablePosition() const 
{
	return stablePosition; 
}

inline const sf::Vector2f& PieceImage::getCurrentPosition() const 
{ 
	return image.getPosition(); 
}

inline sf::FloatRect PieceImage::getGlobalBounds() const 
{
	return image.getGlobalBounds(); 
}

inline Piece PieceImage::piece() const
{
	return representedPiece;
}

inline void PieceImage::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(image, states);
}