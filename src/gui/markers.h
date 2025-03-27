#pragma once

#include "textures.h"
#include <memory>


namespace GUI {

    // ------------------
	// Check marker class
	// ------------------

    // A blured red-circle
    class CheckMarker : public sf::Drawable
    {
    public:
        CheckMarker(float radius, int fragmentation = 20);

        // Position change
        void setPosition(sf::Vector2f pos);

        // Draw
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

    private:
        void alignCircles();

        // Graphic content
        std::unique_ptr<sf::CircleShape[]> circles;

        // Additional info
        const int noCircles;
        const float maxRadius;
    };

}