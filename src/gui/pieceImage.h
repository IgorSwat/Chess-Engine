#pragma once

#include "textures.h"


namespace GUI {

    class PieceImage : public sf::Drawable
    {
    public:
        PieceImage(Piece piece, float size);

        // Image position change
        void setTempPosition(sf::Vector2f pos);     // Change image position, but do not affect stable position (= keep the previous one)
        void setPermPosition(sf::Vector2f pos);     // Change both image and stable position
        void restoreStablePosition();
        void updateOrigin(float mouseX, float mouseY);   // Updates origin of grabbed piece to match mouse posiiton well

        // Draw
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

        // Getters
        sf::Vector2f getCurrentPosition() const { return image.getPosition(); }
        sf::Vector2f getStablePosition() const { return stablePosition; }
        sf::FloatRect getGlobalBounds() const { return image.getGlobalBounds(); }
        Piece representedPiece() const {return piece; }

    private:
        // Graphic content
        sf::Sprite image;
        sf::Vector2f stablePosition;    // Last known correct position, before piece has been grabbed by user

        // Additional info
        Piece piece;
    };

}