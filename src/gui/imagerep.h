#pragma once

#include "pieceimg.h"
#include <array>
#include <vector>
#include <memory>


namespace GUI {

    // --------------------------
	// PieceImageRepository class
	// --------------------------

    class PieceImageRepository
    {
    public:
        // Helper class
        struct ImageBank
        {
            std::vector< std::unique_ptr<PieceImage> > images;
            int used = 0;
        };

        PieceImageRepository(float imageSize = 100.f) : imageSize(imageSize) {}

        // Repository handlers
        PieceImage* takeImage(Piece piece);
        void returnImage(Piece piece);
        void returnAllImages(Piece piece);

    private:
        std::array<ImageBank, PIECE_RANGE> repository;
        float imageSize;
    };

}