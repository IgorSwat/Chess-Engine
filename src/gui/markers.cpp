#include "markers.h"
#include <cmath>


namespace GUI {

    // Customizable parameters
    const sf::Color MARKER_COLOR = sf::Color::Red;


    // --------------------
	// CheckMarker methods
	// --------------------

    CheckMarker::CheckMarker(float radius, int fragmentation)
        : circles(new sf::CircleShape[fragmentation]), noCircles(fragmentation), maxRadius(radius)
    {
        for (int i = 0; i < noCircles; i++) {
            circles[i] = sf::CircleShape(maxRadius - i * maxRadius / noCircles);
            circles[i].setFillColor(sf::Color(MARKER_COLOR.r, MARKER_COLOR.g, MARKER_COLOR.b, 255 * (i + 1) / noCircles));
        }
        alignCircles();
    }

    void CheckMarker::setPosition(sf::Vector2f pos)
    {
        circles[0].setPosition(pos);
        alignCircles();
    }

    void CheckMarker::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        for (int i = 0; i < noCircles; i++)
            target.draw(circles[i], states);
    }

    void CheckMarker::alignCircles()
    {
        sf::Vector2f origin = circles[0].getPosition();

        for (int i = 1; i < noCircles; i++) {
            float xPos = origin.x + i * maxRadius * sqrt(2.f) / (2.f * noCircles);
            float yPos = origin.y + i * maxRadius * sqrt(2.f) / (2.f * noCircles);
            circles[i].setPosition(xPos, yPos);
        }
    }

}