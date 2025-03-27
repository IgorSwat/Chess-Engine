#include "inputbar.h"
#include <iostream>
#include <numeric>
#include <exception>


namespace GUI {

    // -----------------------
	// Customizable parameters
	// -----------------------

    // Background
    const sf::Color BACKGROUND_COLOR = sf::Color::White;
    const sf::Color BACKGROUND_OUTLINE_COLOR = sf::Color::Black;
    const float BACKGROUND_OUTLINE_THICKNESS = 2.f;

    // Text
    const std::string FONT_FILEPATH = "resource/arial.ttf";
    const float DEFAULT_TEXT_RATIO = 0.8f;
    const sf::Color TEXT_COLOR = sf::Color::Black;

    // Cursor
    const sf::Color CURSOR_COLOR = sf::Color::Blue;
    const float CURSOR_THICKNESS = 2.f;
    const sf::Time CURSOR_FLASH_TIME = sf::seconds(0.5f);


    // -----------------------
	// InputBar methods - init
	// -----------------------

    InputBar::InputBar(sf::Vector2f pos, sf::Vector2f size, unsigned charSize)
    {
        // Background setup
        background.setSize(size);
        background.setPosition(pos);
        background.setFillColor(BACKGROUND_COLOR);
        background.setOutlineColor(BACKGROUND_OUTLINE_COLOR);
        background.setOutlineThickness(BACKGROUND_OUTLINE_THICKNESS);

        // Font & text setup
        if (!font.loadFromFile(FONT_FILEPATH))
            throw std::runtime_error("Unable to load font from: " + FONT_FILEPATH);
        text.setFont(font);
        if (charSize == 0)
            charSize = static_cast<unsigned>(size.y * DEFAULT_TEXT_RATIO);
        text.setCharacterSize(charSize);
        text.setFillColor(TEXT_COLOR);
        text.setPosition({pos.x + charSize, pos.y + (size.y - charSize) / 3.f});
        text.setString("X");
        textTop = text.getGlobalBounds().top;
        text.setString("");

        // Cursor setup
        cursor.setSize(sf::Vector2f(CURSOR_THICKNESS, charSize));
        cursor.setFillColor(CURSOR_COLOR);
        updateCursor();
    }


    // ---------------------------------
	// InputBar methods - event handling
	// ---------------------------------

    void InputBar::update(const sf::Event& event, sf::Vector2i mousePos)
    {
        // Mouse click - check focus and update cursor position if needed
        if (event.type == sf::Event::MouseButtonPressed && int(event.key.code) == int(sf::Mouse::Left)) {
            focus = background.getGlobalBounds().contains(sf::Vector2f(mousePos));
            if (focus)
                setCursorPosition(sf::Vector2f(mousePos));
            return;
        }
        // User input change - update input text
        if (focus && event.type == sf::Event::TextEntered) {
            if (event.text.unicode == 8 && !input.empty() && cursorPos > 0) {    // Backspace
                input.erase(cursorPos - 1, 1);  // Remove the previous (vehind cursor) char
                cursorPos--;
                updateText();
            }
            else if (event.text.unicode >= 32 && event.text.unicode < 128) {    // Printable character
                input.insert(cursorPos, 1, static_cast<char>(event.text.unicode));
                cursorPos++;
                updateText();
            }
        }
        // Control keys & CTRL + V
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Left && cursorPos > 0) {    // Left arrow, cursor rewind
                cursorPos--;
                updateCursor();
            }
            else if (event.key.code == sf::Keyboard::Right && cursorPos < input.size()) {   // Right arrow, cursor push
                cursorPos++;
                updateCursor();
            }
            else if (event.key.code == sf::Keyboard::V && event.key.control) {  // CTRL + V for paste
                sf::String clipboardText = sf::Clipboard::getString();
                if (!clipboardText.isEmpty()) {
                    input.insert(cursorPos, std::string(clipboardText));
                    cursorPos += clipboardText.getSize();
                    updateText();
                }
            }
        }

    }


    // -------------------------
	// InputBar methods - render
	// -------------------------

    void InputBar::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(background);
        target.draw(text);
        if (focus && cursorState)
            target.draw(cursor);
    }


    // -----------------------------------
	// InputBar methods - component update
	// -----------------------------------

    void InputBar::updateTimer()
    {
        cursorTimer += clock.restart();
        if (cursorTimer >= CURSOR_FLASH_TIME) {
            cursorTimer = sf::Time::Zero;
            cursorState = !cursorState;
        }
    }

    void InputBar::updateText()
    {
        text.setString(input);
        updateCursor();
    }

    void InputBar::updateCursor() 
    {
        cursor.setPosition({text.findCharacterPos(cursorPos).x, textTop});
    }

    void InputBar::setCursorPosition(sf::Vector2f mousePos)
    {
        // Perform binary search on input text
        std::size_t left = 0, right = input.size();
        while (left < right) {
            std::size_t mid = (left + right) / 2;
            if (mousePos.x > text.findCharacterPos(mid).x)
                left = mid + 1;
            else
                right = mid;
        }

        cursorPos = left;
        updateCursor();
    }

}