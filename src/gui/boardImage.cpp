#include "../logic/boardConfig.h"
#include "../engine/evaluation.h"
#include "boardImage.h"

using namespace BoardTextures;

namespace {
    // Customizable constants
	constexpr float TILE_SIZE = 100.f;
    constexpr int CIRCLES_PER_BLUR = 30;

    constexpr int MAX_PIECES_TYPES[PIECE_TYPE_RANGE] = { 0, 8, 12, 12, 10, 9, 1 };
    constexpr int PAWN_STARTING_RANK[COLOR_RANGE] = { 1, 6 };
    constexpr Movemask PROMOTION_MASKS[PROMOTION_CHOICE_RANGE] = {
        QUEEN_PROMOTION_FLAG, ROOK_PROMOTION_FLAG, BISHOP_PROMOTION_FLAG, KNIGHT_PROMOTION_FLAG
    };

    sf::Vector2f squareToVector(Square sq)
    {
        return sf::Vector2f(TILE_SIZE * file_of(sq), TILE_SIZE * (7 -rank_of(sq)));
    }
}


BoardImage::BoardImage(BoardController* boardController, Evaluation::Evaluator* evaluator)
    : controller(boardController), evaluator(evaluator),
      piecesTables(PIECE_RANGE), squaresTexture(BoardTextures::getTexture("squares")),
      movedTo(sf::Vector2f(TILE_SIZE, TILE_SIZE)), movedFrom(sf::Vector2f(TILE_SIZE, TILE_SIZE)),
      checkBlur(TILE_SIZE / 1.9f, CIRCLES_PER_BLUR),
      promotionFog(sf::Vector2f(TILE_SIZE * 8, TILE_SIZE * 8)),
      whitePromotionBar(WHITE, TILE_SIZE), blackPromotionBar(BLACK, TILE_SIZE)
{
    // Loading the squares
    constexpr int verticesNum = 8 * 8 * 4;
    squareGrid.setPrimitiveType(sf::Quads);
    squareGrid.resize(verticesNum);
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int id = (i + j) % 2;
            sf::Vertex* quad = &squareGrid[(i * 8 + j) * 4];
            quad[0].position = sf::Vector2f(i * TILE_SIZE, j * TILE_SIZE);
            quad[1].position = sf::Vector2f((i + 1) * TILE_SIZE, j * TILE_SIZE);
            quad[2].position = sf::Vector2f((i + 1) * TILE_SIZE, (j + 1) * TILE_SIZE);
            quad[3].position = sf::Vector2f(i * TILE_SIZE, (j + 1) * TILE_SIZE);
            quad[0].texCoords = sf::Vector2f(id * 100.f, 0.f);
            quad[1].texCoords = sf::Vector2f((id + 1) * 100.f, 0.f);
            quad[2].texCoords = sf::Vector2f((id + 1) * 100.f, 100.f);
            quad[3].texCoords = sf::Vector2f(id * 100.f, 100.f);
        }
    }

    // Initialising pieces vectors
	pieces.reserve(GUI::MAX_PIECES_NUM);
	for (int p = W_PAWN; p <= W_KING; p++) {
		Piece whitePiece = Piece(p);
		Piece blackPiece = Piece(p | BLACK_PIECE);
        PieceType type = type_of(whitePiece);
        for (int i = 0; i < MAX_PIECES_TYPES[type]; i++) {
            piecesTables[whitePiece].pieces.emplace_back(whitePiece, BoardTextures::pieceTexture(whitePiece));
            piecesTables[blackPiece].pieces.emplace_back(blackPiece, BoardTextures::pieceTexture(blackPiece));
        }
	}

    // Other graphical enteties
    movedFrom.setFillColor(sf::Color(224, 247, 106, 100));
    movedTo.setFillColor(sf::Color(214, 245, 61, 140));
    promotionFog.setPosition(0.f, 0.f);
    promotionFog.setFillColor(sf::Color(71, 74, 72, 120));
}

void BoardImage::loadPosition(const BoardConfig* board)
{
    clearPosition();
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            sf::Vector2f screenPos = { TILE_SIZE * j, TILE_SIZE * i };
            Piece piece = board->onSquare(make_square(7 - i, j));
            if (piece != NO_PIECE) {
                PieceImage* pieceImage = piecesTables[piece].top();
                pieceImage->setPermPosition(screenPos);
                pieces.push_back(pieceImage);
            }
        }
    }
    updateCheckBlur(board);
    showEvaluationStats(board);
}

void BoardImage::updatePieces(BoardConfig* board)
{
    Color sideOnMove = board->movingSide();
    bool isMousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
    if (!mouseLeftButtonClicked && selectedPiece == nullptr && isMousePressed) {
        for (PieceImage* pieceImage : pieces) {
            if (color_of(pieceImage->piece()) != sideOnMove)
                continue;
            sf::FloatRect bounds = pieceImage->getGlobalBounds();
            if (bounds.contains(float(mousePos.x), float(mousePos.y))) {
                selectedPiece = pieceImage;
                selectedPiece->updateOrigin(mousePos);
                selectedPiece->setTempPosition(sf::Vector2f(float(mousePos.x), float(mousePos.y)));
                break;
            }
        }
    }
    else if (selectedPiece != nullptr && isMousePressed)
        selectedPiece->setTempPosition(sf::Vector2f(float(mousePos.x), float(mousePos.y)));
    else if (selectedPiece != nullptr && !isMousePressed) {
        sf::Vector2f lastPos = selectedPiece->getStablePosition();
        int rank = 7 - int(mousePos.y / TILE_SIZE);
        int file = mousePos.x / TILE_SIZE;
        if (rank >= 0 && rank < 8 && file >= 0 && file < 8) {
            Square to = make_square(rank, file);
            Square from = make_square(7 - int(lastPos.y / TILE_SIZE), int(lastPos.x / TILE_SIZE));
            Move move = translateMove(from, to, selectedPiece->piece(), board->onSquare(to));
            if (move != NULL_MOVE && board->legalityCheckFull(move)) { // Test for legality
                sf::Vector2f oldPos = selectedPiece->getStablePosition();
                sf::Vector2f newPos(file * TILE_SIZE, (7 - rank) * TILE_SIZE);
                if (move.isPromotion()) {
                    promotionMove = move;
                    selectedPromotionBar = sideOnMove == WHITE ? &whitePromotionBar : &blackPromotionBar;
                    selectedPromotionBar->setPosition(newPos, sideOnMove == WHITE);  // Fix for black
                    awaitingPromotion = true;
                }
                else {
                    board->makeMove(move);
                    loadPosition(board);
                    moveRectanglesState = true;
                }
                movedFrom.setPosition(oldPos);
                movedTo.setPosition(newPos);
            }
            else
                selectedPiece->resetToStablePosition();
        }
        else
            selectedPiece->resetToStablePosition();
        selectedPiece = nullptr;
    }
}

void BoardImage::updatePromotion(BoardConfig* board)
{
    PromotionChoice choice = selectedPromotionBar->update();
    if (choice == PromotionChoice::NONE) 
        return;
    Movemask flags = promotionMove.flags() | PROMOTION_MASKS[int(choice)];
    Move move = { promotionMove.from(), promotionMove.to(), flags };
    board->makeMove(move);
    loadPosition(board);
    awaitingPromotion = false;
    moveRectanglesState = true;
}

void BoardImage::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.texture = &squaresTexture;
    target.draw(squareGrid, states);

    if (moveRectanglesState) {
        target.draw(movedFrom);
        target.draw(movedTo);
    }
    if (bluredCircleState)
        target.draw(checkBlur);

    for (PieceImage* piece : pieces) {
        if (selectedPiece != piece)
            target.draw(*piece);
    }
    if (selectedPiece != nullptr)
        target.draw(*selectedPiece);

    if (awaitingPromotion)
        target.draw(promotionFog);
    if (awaitingPromotion)
        target.draw(*selectedPromotionBar);
}

void BoardImage::clearPosition()
{
    pieces.clear();
    for (int p = W_PAWN; p <= W_KING; p++) {
        Piece whitePiece = Piece(p);
        Piece blackPiece = Piece(p | BLACK_PIECE);
        piecesTables[whitePiece].topID = 0;
        piecesTables[blackPiece].topID = 0;
    }
    moveRectanglesState = false;
    bluredCircleState = false;
}

void BoardImage::updateCheckBlur(const BoardConfig* board)
{
    Square checkedKingSq = INVALID_SQUARE;
    if (board->isInCheck(WHITE))
        checkedKingSq = board->kingPosition(WHITE);
    if (board->isInCheck(BLACK))
        checkedKingSq = board->kingPosition(BLACK);
    if (checkedKingSq != INVALID_SQUARE) {
        checkBlur.setPosition(squareToVector(checkedKingSq));
        bluredCircleState = true;
    }
    else
        bluredCircleState = false;
}

Move BoardImage::translateMove(Square from, Square to, Piece piece, Piece onTargetSquare) const
{
    PieceType type = type_of(piece);
    Color color = color_of(piece);

    if (type == PAWN) {
        int rankDiff1 = color == WHITE ? 1 : -1;
        int rankDiff2 = rankDiff1 * 2;
        bool promotion = rank_of(to) == 0 || rank_of(to) == 7;
        if (file_of(from) == file_of(to) && rank_of(to) - rank_of(from) == rankDiff1)
            return promotion ? Move(from, to, PROMOTION_FLAG) : Move(from, to, QUIET_MOVE_FLAG);
        if (file_of(from) == file_of(to) && rank_of(to) - rank_of(from) == rankDiff2)
            return rank_of(from) == PAWN_STARTING_RANK[color] ? Move(from, to, DOUBLE_PAWN_PUSH_FLAG) : NULL_MOVE;
        if (!(Pieces::pawn_attacks(color, from) & to))
            return NULL_MOVE;
        return onTargetSquare != NO_PIECE ? (promotion ? Move(from, to, CAPTURE_FLAG | PROMOTION_FLAG) : Move(from, to, CAPTURE_FLAG)) : 
            Move(from, to, ENPASSANT_FLAG);
    }

    if (type == KING && rank_of(from) == rank_of(to) && (from == to - 2 || from == to + 2))
        return to == from - 2 ? Move(from, to, QUEENSIDE_CASTLE_FLAG) : Move(from, to, KINGSIDE_CASTLE_FLAG);


    if (!Pieces::in_piece_range(type, from, to))
        return NULL_MOVE;
    return onTargetSquare != NO_PIECE ? Move(from, to, CAPTURE_FLAG) : Move(from, to, QUIET_MOVE_FLAG);
}

void BoardImage::showEvaluationStats(const BoardConfig* board)
{
    system("cls");
    Value eval = evaluator->evaluate();
    std::cout << "Total eval: " << eval << std::endl;
    //std::cout << "Zobrist: " << std::hex << board->hash() << std::endl;
}