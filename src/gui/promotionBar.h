#pragma once

#include "textures.h"
#include <vector>


enum class PromotionChoice : int {
	QUEEN = 0, ROOK, BISHOP, KNIGHT,
	NONE
};

constexpr int PROMOTION_CHOICE_RANGE = 4;



struct PromotionEntry
{
	sf::CircleShape circle;
	sf::Sprite sprite;
};



class PromotionBar : public sf::Drawable
{
public:
	PromotionBar(Color side, float tileSize);
	
	void setPosition(const sf::Vector2f& pos, bool topCorner = true);
	void setPosition(float xPos, float yPos, bool topCorner = true);

	PromotionChoice update();	// Returns the piece selected as a promotion result

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	void setAlignment();

	sf::Vector2f topLeftCorner;
	float entrySize;

	std::vector<PromotionEntry> entries;

};



inline void PromotionBar::setPosition(const sf::Vector2f& pos, bool topCorner)
{
	topLeftCorner = topCorner ? pos : pos - sf::Vector2f(0.f, entrySize * (entries.size() - 1));
	setAlignment();
}

inline void PromotionBar::setPosition(float xPos, float yPos, bool topCorner)
{
	setPosition(sf::Vector2f(xPos, yPos), topCorner);
}

inline void PromotionBar::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	for (const PromotionEntry& entry : entries) {
		target.draw(entry.circle, states);
		target.draw(entry.sprite, states);
	}
}