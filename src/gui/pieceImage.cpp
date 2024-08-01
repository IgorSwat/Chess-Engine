#include "pieceImage.h"


namespace GUI {

    PieceImage::PieceImage(Piece piece, sf::Vector2f initialPos)
        : image(Textures::get_texture(piece)), stablePosition(initialPos), piece(piece)
    {
        image.setPosition(stablePosition);
    }


    // ---------------
	// Position change
	// ---------------

    void PieceImage::setTempPosition(sf::Vector2f pos)
    {
        image.setPosition(pos);
    }

    void PieceImage::setPermPosition(sf::Vector2f pos)
    {
        stablePosition = pos;
        restoreStablePosition();
    }

    void PieceImage::restoreStablePosition()
    {
        image.setOrigin({ 0.f, 0.f });
	    image.setPosition(stablePosition);
    }

    void PieceImage::updateOrigin(float mouseX, float mouseY)
    {
        image.setOrigin(mouseX - stablePosition.x, mouseY - stablePosition.y);
    }


    // -------
	// Drawing
	// -------

    void PieceImage::draw(sf::RenderTarget &target, sf::RenderStates states) const
    {
        target.draw(image, states);
    }
    
}