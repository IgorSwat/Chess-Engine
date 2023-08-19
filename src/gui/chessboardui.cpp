#include <cstdlib>
#include <iostream>
#include "chessboardui.h"
#include "../core/chessboard.h"

using sf::Vector2f;
using BoardTextures::textures;

void ChessboardUI::loadSquares()
{
    vertices.setPrimitiveType(sf::Quads);
    vertices.resize(8 * 8 * 4);
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            int id = (i + j) % 2;
            sf::Vertex* quad = &vertices[(i * 8 + j) * 4];
            quad[0].position = sf::Vector2f(i * tileSize, j * tileSize);
            quad[1].position = sf::Vector2f((i + 1) * tileSize, j * tileSize);
            quad[2].position = sf::Vector2f((i + 1) * tileSize, (j + 1) * tileSize);
            quad[3].position = sf::Vector2f(i * tileSize, (j + 1) * tileSize);
            quad[0].texCoords = sf::Vector2f(id * 100.f, 0.f);
            quad[1].texCoords = sf::Vector2f((id + 1) * 100.f, 0.f);
            quad[2].texCoords = sf::Vector2f((id + 1) * 100.f, 100.f);
            quad[3].texCoords = sf::Vector2f(id * 100.f, 100.f);
        }
    }
}

void ChessboardUI::initSprites()
{
    pieces.push_back(PieceImage(textures["Wking"], sf::Vector2f(tileSize * 4.f, tileSize * 7.f)));
    for (int i = 0; i < 8; i++)
        pieces.push_back(PieceImage(textures["Wpawn"], sf::Vector2f(i * tileSize, tileSize * 6.f)));
    pieces.push_back(PieceImage(textures["Wknight"], sf::Vector2f(tileSize, tileSize * 7.f)));
    pieces.push_back(PieceImage(textures["Wknight"], sf::Vector2f(tileSize * 6.f, tileSize * 7.f)));
    pieces.push_back(PieceImage(textures["Wbishop"], sf::Vector2f(tileSize * 2.f, tileSize * 7.f)));
    pieces.push_back(PieceImage(textures["Wbishop"], sf::Vector2f(tileSize * 5.f, tileSize * 7.f)));
    pieces.push_back(PieceImage(textures["Wrook"], sf::Vector2f(0.f, tileSize * 7.f)));
    pieces.push_back(PieceImage(textures["Wrook"], sf::Vector2f(tileSize * 7.f, tileSize * 7.f)));
    pieces.push_back(PieceImage(textures["Wqueen"], sf::Vector2f(tileSize * 3.f, tileSize * 7.f)));

    pieces.push_back(PieceImage(textures["Bking"], sf::Vector2f(tileSize * 4.f, 0.f)));
    for (int i = 0; i < 8; i++)
        pieces.push_back(PieceImage(textures["Bpawn"], sf::Vector2f(i * tileSize, tileSize)));
    pieces.push_back(PieceImage(textures["Bknight"], sf::Vector2f(tileSize, 0.f)));
    pieces.push_back(PieceImage(textures["Bknight"], sf::Vector2f(tileSize * 6.f, 0.f)));
    pieces.push_back(PieceImage(textures["Bbishop"], sf::Vector2f(tileSize * 2.f, 0.f)));
    pieces.push_back(PieceImage(textures["Bbishop"], sf::Vector2f(tileSize * 5.f, 0.f)));
    pieces.push_back(PieceImage(textures["Brook"], sf::Vector2f(0.f, 0.f)));
    pieces.push_back(PieceImage(textures["Brook"], sf::Vector2f(tileSize * 7.f, 0.f)));
    pieces.push_back(PieceImage(textures["Bqueen"], sf::Vector2f(tileSize * 3.f, 0.f)));
}

ChessboardUI::ChessboardUI(Chessboard* board, float tsize)
    : redCircle(tsize / 1.9f, circleCount),
      movedTo(sf::Vector2f(tsize, tsize)),
      movedFrom(sf::Vector2f(tsize, tsize)),
      fog(sf::Vector2f(tsize * 8, tsize * 8)), whitePromotionBar(WHITE),
      blackPromotionBar(BLACK), arrow(tsize)
{
    logicBoard = board;
    tileSize = tsize;
    currentPromotionBar = nullptr;
    if (!squaresTexture.loadFromFile("resource/squares.png"))
    {
        std::cout<<"Bad texture path for squares texture.\n";
        exit(-1);
    }
    loadSquares();
    initSprites();
    fog.setPosition(0.f, 0.f);
    fog.setFillColor(sf::Color(71, 74, 72, 120));
    movedFrom.setFillColor(sf::Color(224, 247, 106, 100));
    movedTo.setFillColor(sf::Color(214, 245, 61, 140));
    arrow.setColor(sf::Color(180, 207, 240, 180));
}

void ChessboardUI::setTextureFor(PieceImage& piece, Side color, PieceType type)
{
    std::string key = "";
    if (color == WHITE)
        key += "W";
    else
        key += "B";
    switch (type)
    {
    case PieceType::PAWN:
        key += "pawn";
        break;
    case KNIGHT:
        key += "knight";
        break;
    case BISHOP:
        key += "bishop";
        break;
    case ROOK:
        key += "rook";
        break;
    case QUEEN:
        key += "queen";
        break;
    case PieceType::KING:
        key += "king";
        break;
    default:
        key += "undefined";
        break;
    }
    piece.setTexture(textures[key]);
}

void ChessboardUI::updateRedCircle(BoardConfig* config)
{
    Piece* checkedKing = config->getKingUnderCheck();
    if (checkedKing != nullptr)
    {
        redCircleState = true;
        Square pos = checkedKing->getPos();
        redCircle.setPosition(pos.x * tileSize, pos.y * tileSize);
    }
    else
        redCircleState = false;
}

void ChessboardUI::placePromotionBar()
{
    const sf::Vector2f& pos = pieces[awaitingPromotion].getCurrentPosition();
    sf::Vector2f posNormalized(tileSize * static_cast<int>(pos.x / tileSize), tileSize * static_cast<int>(pos.y / tileSize));
    if (posNormalized.y < 1.f)
    {
        currentPromotionBar = &whitePromotionBar;
        currentPromotionBar->setPosition(posNormalized);
    }
    else
    {
        currentPromotionBar = &blackPromotionBar;
        currentPromotionBar->setPosition(posNormalized.x, posNormalized.y - tileSize * 3);
    }
}

void ChessboardUI::setArrow(int pieceID, const Square& targetSquare)
{
    const sf::Vector2f& piecePos = pieces[pieceID].getStablePosition();
    sf::Vector2f initPos(piecePos.x + tileSize / 2, piecePos.y + tileSize / 2);
    sf::Vector2f targetPos(static_cast<float>(targetSquare.x * tileSize + tileSize / 2),
                           static_cast<float>(targetSquare.y * tileSize + tileSize / 2));
    arrow.matchToSquares(initPos, targetPos);
    arrowState = true;
}

void ChessboardUI::moveSprite(int spriteID, const Square& pos)
{
    PieceImage& sprite = pieces[spriteID];
    sprite.setPermPosition(Vector2f((float)pos.x * tileSize, (float)pos.y * tileSize));
}

void ChessboardUI::updateSprites(sf::RenderWindow* window)
{
    if (!mouseState && selectedPiece == nullptr && sf::Mouse::isButtonPressed(sf::Mouse::Left))
    {
        for (int i = colorID; i < colorID + 16; i++)
        {
            PieceImage& piece = pieces[i];
            if (!piece.isActive())
                continue;
            sf::FloatRect bounds = piece.getGlobalBounds();
            sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
            if (mousePos.x > bounds.left && mousePos.x < bounds.left + bounds.width &&
                mousePos.y > bounds.top && mousePos.y < bounds.top + bounds.height) {
                selectedPiece = &piece;
                selectedPiece->updateOrigin(mousePos);
                selectedPiece->setTempPosition(sf::Vector2f(mousePos.x, mousePos.y));
                break;
            }
        }
        if (selectedPiece == nullptr)
            mouseState = true;
    }
    else if (selectedPiece != nullptr && sf::Mouse::isButtonPressed(sf::Mouse::Left))
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
        selectedPiece->setTempPosition(sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)));
    }
    else if (selectedPiece != nullptr && !sf::Mouse::isButtonPressed(sf::Mouse::Left))
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
        sf::Vector2f lastPos = selectedPiece->getStablePosition();
        int xid = int(mousePos.x / tileSize);
        int yid = int(mousePos.y / tileSize);
        Square targetSquare = Square(xid, yid);
        Square initSquare = Square(int(lastPos.x / tileSize), int(lastPos.y / tileSize));
        try
        {
            if (logicBoard->registerMovement(initSquare, targetSquare))
            {
                moveRectanglesState = true;
                sf::Vector2f oldPos = selectedPiece->getStablePosition();
                sf::Vector2f newPos(xid * tileSize, yid * tileSize);
                selectedPiece->setPermPosition(newPos);
                colorID = (colorID + 16) % 32;
                updateRedCircle(logicBoard->getCurrentConfig());
                movedFrom.setPosition(oldPos);
                movedTo.setPosition(newPos);
                logicBoard->testDynamics();
            }
            else
                selectedPiece->resetToStablePosition();
            selectedPiece = nullptr;
        }
        catch (std::bad_exception&)
        {
            std::cout<<"Error: do not exist any piece on square ["<<initSquare.y<<","<<initSquare.x<<"]"<<std::endl;
            exit(-219);
        }
    }
    if (!sf::Mouse::isButtonPressed(sf::Mouse::Left))
        mouseState = false;
}

PromotionChoice ChessboardUI::updatePromotionBar(sf::RenderWindow* window)
{
    if (currentPromotionBar == nullptr)
        return PromotionChoice::NONE;
    return currentPromotionBar->update(window);
}

void ChessboardUI::loadFromConfig(BoardConfig* config)
{
    for (unsigned int i = 0; i < pieces.size(); i++)
    {
        Piece* piece = config->getPiece(i);
        if (piece->isActive())
        {
            sf::Vector2f position(tileSize * piece->getPos().x, tileSize * piece->getPos().y);
            pieces[i].setPermPosition(position);
            setTextureFor(pieces[i], piece->getColor(), piece->getType());
        }
        pieces[i].setState(piece->isActive());
    }
    updateRedCircle(config);
    if (config->getSideOnMove() == WHITE)
        colorID = 0;
    else
        colorID = 16;
    moveRectanglesState = false;
}

void ChessboardUI::setPromotionFlag(int promotedPieceID)
{
    awaitingPromotion = promotedPieceID;
    if (awaitingPromotion > 0)
        placePromotionBar();
    else
        currentPromotionBar = nullptr;
}

void ChessboardUI::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    states.texture = &squaresTexture;
    target.draw(vertices, states);
    if (moveRectanglesState)
    {
        target.draw(movedFrom);
        target.draw(movedTo);
    }
    if (redCircleState)
        target.draw(redCircle);
    for (const PieceImage& p : pieces)
    {
        if (selectedPiece != &p)
            target.draw(p);
    }
    if (selectedPiece != nullptr)
        target.draw(*selectedPiece);
    if (awaitingPromotion > 0)
        target.draw(fog);
    if (currentPromotionBar != nullptr)
        currentPromotionBar->render(target);
    if (arrowState)
        target.draw(arrow);
}

