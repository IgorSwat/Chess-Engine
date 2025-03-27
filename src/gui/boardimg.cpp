#include "../engine/board.h"
#include "../engine/movegen.h"
#include "boardimg.h"
#include <algorithm>


namespace GUI {

    // Customizable parameters
    constexpr int CHECK_MARKER_FRAGMENTATION = 20;

    constexpr float SQUARE_TEXTURE_SIZE = 96.f;

    const sf::Color MOVED_FROM_MARKER_COLOR = sf::Color(224, 247, 106, 100);
    const sf::Color MOVED_TO_MARKER_COLOR = sf::Color(224, 247, 106, 100);
    const sf::Color PROMOTION_FOG_COLOR = sf::Color(71, 74, 72, 120);


    // -------------------------
	// BoardImage methods - init
	// -------------------------

    BoardImage::BoardImage(float tileSize)
        : tileSize(tileSize),
          imageRepository(tileSize),
          movedFromMarker(sf::Vector2f(tileSize, tileSize)), movedToMarker(sf::Vector2f(tileSize, tileSize)),
          checkMarker(tileSize / 2.f, CHECK_MARKER_FRAGMENTATION),
          promotionFog(sf::Vector2f(tileSize * 8, tileSize * 8)), whitePromotionBar(WHITE, tileSize), blackPromotionBar(BLACK, tileSize)
    {
        constexpr int NO_VERTICES = 8 * 8 * 4;

        // Initialize square grid
        squares.setPrimitiveType(sf::Quads);
        squares.resize(NO_VERTICES);
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                int id = (i + j) % 2;
                sf::Vertex *quad = &squares[(i * 8 + j) * 4];
                quad[0].position = sf::Vector2f(i * tileSize, j * tileSize);
                quad[1].position = sf::Vector2f((i + 1) * tileSize, j * tileSize);
                quad[2].position = sf::Vector2f((i + 1) * tileSize, (j + 1) * tileSize);
                quad[3].position = sf::Vector2f(i * tileSize, (j + 1) * tileSize);
                quad[0].texCoords = sf::Vector2f(id * SQUARE_TEXTURE_SIZE, 0.f);
                quad[1].texCoords = sf::Vector2f((id + 1) * SQUARE_TEXTURE_SIZE, 0.f);
                quad[2].texCoords = sf::Vector2f((id + 1) * SQUARE_TEXTURE_SIZE, SQUARE_TEXTURE_SIZE);
                quad[3].texCoords = sf::Vector2f(id * SQUARE_TEXTURE_SIZE, SQUARE_TEXTURE_SIZE);
            }
        }

        // Initialize other visual elements
        movedFromMarker.setFillColor(MOVED_FROM_MARKER_COLOR);
        movedToMarker.setFillColor(MOVED_TO_MARKER_COLOR);

        promotionFog.setPosition(0.f, 0.f);
        promotionFog.setFillColor(PROMOTION_FOG_COLOR);
    }


    // -------------------------------------
	// BoardImage methods - position loading
	// -------------------------------------

    void BoardImage::loadPosition(const Board* board)
    {
        // Reset board
        clearBoard();

        // Load piece images
        for (int sq = SQ_A1; sq <= SQ_H8; sq++) {
            Piece piece = board->on(Square(sq));
            if (piece != NO_PIECE) {
                PieceImage* image = imageRepository.takeImage(piece);
                image->setPermPosition(squareToCoords(Square(sq)));
                pieces.push_back(image);
            }
        }

        // Load move markers based on last move
        Move lastMove = board->last_move();
        if (lastMove != Moves::null) {
            movedFromMarker.setPosition(squareToCoords(lastMove.from()));
            movedToMarker.setPosition(squareToCoords(lastMove.to()));
            moveMarkerState = true;
        }
        
        // Load check marker
        if (board->in_check()) {
            Square sq = board->king_position(board->side_to_move());
            checkMarker.setPosition(squareToCoords(sq));
            checkMarkerState = true;
        }
        else
            checkMarkerState = false;

    }

    void BoardImage::clearBoard()
    {
        // Remove all active image pointers
        pieces.clear();

        // Reset all piece image banks
        for (int piece = W_PAWN; piece <= W_KING; piece++) {
            imageRepository.returnAllImages(Piece(piece));                  // White pieces
            imageRepository.returnAllImages(Piece(piece | BLACK_PIECE));    // Black pieces
        }

        // Reset other flags and logic
        selectedPiece = nullptr;
        activePromotionBar = nullptr;
        moveMarkerState = false;
        checkMarkerState = false;
    }


    // -------------------------------------
	// BoardImage methods - event processing
	// -------------------------------------

    std::pair<Move, bool> BoardImage::update(const sf::Event& event, sf::Vector2i mousePos, const Board* board)
    {
        // Case 1 - no promotion awaiting, just moving pieces
        if (activePromotionBar == nullptr) {
            // Mouse click, try to select some piece
            if (event.type == sf::Event::MouseButtonPressed && int(event.key.code) == int(sf::Mouse::Left) && !selectedPiece) {
                auto selected = std::find_if(pieces.begin(), pieces.end(), [&mousePos](PieceImage* image){
                    return image->getGlobalBounds().contains(sf::Vector2f(mousePos));
                });
                if (selected != pieces.end() && color_of((*selected)->representedPiece()) == board->side_to_move()) {
                    selectedPiece = *selected;
                    selectedPiece->updateOrigin(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
                    selectedPiece->setTempPosition(sf::Vector2f(mousePos));
                }
            }
            // Mouse button released, try to place selected piece (if exists) on new square
            else if (event.type == sf::Event::MouseButtonReleased && selectedPiece) {
                Square from = coordsToSquare(selectedPiece->getStablePosition());
                Square to = coordsToSquare(selectedPiece->getCurrentPosition());
                if (from != to) {
                    // Create move and return it
                    return { MoveGeneration::create_move(*board, from, to), false };
                }
                else {
                    selectedPiece->restoreStablePosition();
                    selectedPiece = nullptr;
                }
            }
            // Move the selected piece
            else if (event.type == sf::Event::MouseMoved && selectedPiece)
                selectedPiece->setTempPosition(sf::Vector2f(mousePos));
        }
        else {
            PieceType promoteTo = activePromotionBar->update(event, mousePos);
            if (promoteTo != NULL_PIECE_TYPE) {
                activePromotionBar = nullptr;
                moveMarkerState = true;
                Moves::Flags promotionMask = promoteTo == QUEEN ? Moves::QUEEN_PROMOTION_FLAG :
                                             promoteTo == ROOK ? Moves::ROOK_PROMOTION_FLAG :
                                             promoteTo == KNIGHT ? Moves::KNIGHT_PROMOTION_FLAG : Moves::BISHOP_PROMOTION_FLAG;
                // Return correct promotion move
                return { Move(awaitingPromotion.from(), awaitingPromotion.to(), awaitingPromotion.flags() | promotionMask), true };
            }
        }

        return { Moves::null, false };
    }

    void BoardImage::setupPromotionScreen(Move incompletePromotion)
    {
        Square to = incompletePromotion.to();

        awaitingPromotion = incompletePromotion;
        activePromotionBar = rank_of(to) == 7 ? &whitePromotionBar : &blackPromotionBar;
        activePromotionBar->setPosition(squareToCoords(to));
        moveMarkerState = false;
    }


    // ----------------------------------
	// BoardImage methods - visualization
	// ----------------------------------

    void BoardImage::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // Draw squares
        states.texture = &Textures::get_texture("squares");
        target.draw(squares, states);

        // Draw markers
        if (moveMarkerState) {
            target.draw(movedFromMarker);
            target.draw(movedToMarker);
        }
        if (checkMarkerState)
            target.draw(checkMarker);

        // Draw pieces - with selected piece at the top of piece z-index
        for (PieceImage* image : pieces) {
            if (image != selectedPiece)
                target.draw(*image);
        }
        if (selectedPiece != nullptr)
            target.draw(*selectedPiece);
        
        // Draw promotion elements
        if (activePromotionBar != nullptr) {
            target.draw(promotionFog);
            target.draw(*activePromotionBar);
        }
    }


    // -------------------------------------
	// BoardImage methods - helper functions
	// -------------------------------------

    sf::Vector2f BoardImage::squareToCoords(Square sq) const
    {
        return sf::Vector2f(
            tileSize * float(file_of(sq)),         // x
            tileSize * float((7 - rank_of(sq)))    // y
        );
    }

    Square BoardImage::coordsToSquare(sf::Vector2f coords) const
    {
        return make_square(
            Rank(std::clamp(7 - int(coords.y / tileSize), 0, 7)),
            File(std::clamp(int(coords.x / tileSize), 0, 7))
        );
    }

}