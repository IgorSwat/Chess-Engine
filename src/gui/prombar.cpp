#include "prombar.h"
#include "../engine/types.h"


namespace GUI {

    // Customizable parameters
    constexpr float ICON_SCALE_FACTOR = 0.8f;

    const sf::Color COLOR_DEFAULT = sf::Color(115, 119, 128);
    const sf::Color COLOR_ON_HOVER = sf::Color(149, 151, 158);
    const sf::Color COLOR_ON_CLICK = sf::Color(190, 202, 232);


    // ----------------------
	// PromotionEntry methods
	// ----------------------

    void PromotionBar::PromotionEntry::onDefault()
    {
        circle.setFillColor(COLOR_DEFAULT);
    }

    void PromotionBar::PromotionEntry::onHover()
    {
        circle.setFillColor(COLOR_ON_HOVER);
    }

    void PromotionBar::PromotionEntry::onClick()
    {
        circle.setFillColor(COLOR_ON_CLICK);
    }


    // --------------------
	// PromotionBar methods
	// --------------------

    PromotionBar::PromotionBar(Color side, float tileSize)
        : side(side), entrySize(tileSize)
    {
        const float radius = entrySize / 2.f;

        for (int i = 0; i < entries.size(); i++) {
            PieceType promotionType = PieceType(QUEEN - i);

            entries[i].circle = sf::CircleShape(radius);
            entries[i].icon.setTexture(Textures::get_texture(make_piece(side, promotionType)));
            entries[i].icon.setScale(ICON_SCALE_FACTOR, ICON_SCALE_FACTOR);
        }
    }

    void PromotionBar::setPosition(sf::Vector2f pos)
    {
        topLeft = side == WHITE ? pos : pos - sf::Vector2f(0.f, entrySize * (entries.size() - 1));
        alignEntries();
    }

    PieceType PromotionBar::update(const sf::Event& event, sf::Vector2i mousePos)
    {
        PieceType result = NULL_PIECE_TYPE;

        // PromotionBar accepts only mouse left clicks
		if (event.type == sf::Event::MouseButtonPressed && int(event.key.code) != int(sf::Mouse::Left))
			return result;

        for (int i = 0; i < entries.size(); i++) {
            if (entries[i].circle.getGlobalBounds().contains(sf::Vector2f(mousePos))) {
                if (entries[i].update(event.type, true))
                    result = PieceType(QUEEN - i);
            }
            else
                entries[i].update(sf::Event::MouseMoved, false);
        }

        return result;
    }

    void PromotionBar::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        for (const PromotionEntry& entry : entries) {
            target.draw(entry.circle, states);
            target.draw(entry.icon, states);
        }
    }

    void PromotionBar::alignEntries()
    {
        const float adjustment = (1.f - ICON_SCALE_FACTOR) * entrySize / 2.f;

        for (int i = 0; i < entries.size(); i++) {
            sf::Vector2f pos = {topLeft.x, topLeft.y + i * entrySize};

            entries[i].circle.setPosition(pos);
            entries[i].icon.setPosition(pos + sf::Vector2f(adjustment, adjustment));
        }
    }

}