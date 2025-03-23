#include "imagerep.h"


namespace GUI {

    // ----------------------------
	// PieceImageRepository methods
	// ----------------------------

    PieceImage* PieceImageRepository::takeImage(Piece piece)
    {
        ImageBank& bank = repository[piece];

        // Check if new image needs to be generated
        if (bank.used == bank.images.size())
            bank.images.push_back(std::make_unique<PieceImage>(piece, imageSize));
        
        PieceImage* image = bank.images[bank.used].get();
        bank.used++;

        return image;
    }

    void PieceImageRepository::returnImage(Piece piece)
    {
        repository[piece].used--;
    }

    void PieceImageRepository::returnAllImages(Piece piece)
    {
        repository[piece].used = 0;
    }

}