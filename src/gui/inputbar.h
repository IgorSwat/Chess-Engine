#pragma once

#include "textures.h"


namespace GUI {

    // -------------- 
	// InputBar class
	// --------------

    class InputBar : public sf::Drawable
    {
    public:
        InputBar(sf::Vector2f pos, sf::Vector2f size, unsigned charSize = 0);

        void clear() { input = ""; updateText(); }
        void update(const sf::Event& event, sf::Vector2i mousePos);
        void updateTimer();     // Needs to be called in main loop, before rendering

        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

        // Getters
        std::string getInput() const { return input; }

    private:
        void updateText();
        void updateCursor();
        void setCursorPosition(sf::Vector2f mousePos);

        // Graphic content
        sf::RectangleShape background;
        sf::Font font;
        sf::Text text;
        sf::RectangleShape cursor;

        // Logic - text
        std::string input = "";
        bool focus = false;
        float textTop = 0.f;

        // Logic - cursor
        std::size_t cursorPos = 0;
        sf::Clock clock;
        sf::Time cursorTimer;
        bool cursorState = false;
    };

}