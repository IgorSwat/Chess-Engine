#include <exception>
#include <iostream>
#include "chessboard.h"

using sf::Vector2i;

void Chessboard::initTextures()
{
    textures["back"] = new sf::Texture();
    if (!textures["back"]->loadFromFile("resource/back.png"))
    {
        std::cout<<"Bad texture path.\n";
        exit(-1);
    }
    textures["back"]->setSmooth(true);
    textures["refresh"] = new sf::Texture();
    if (!textures["refresh"]->loadFromFile("resource/refresh.png"))
    {
        std::cout<<"Bad texture path.\n";
        exit(-1);
    }
    textures["refresh"]->setSmooth(true);
    textures["Wking"] = new sf::Texture();
    if (!textures["Wking"]->loadFromFile("resource/Wking.png"))
    {
        std::cout<<"Bad texture path.\n";
        exit(-1);
    }
    textures["Bking"] = new sf::Texture();
    if (!textures["Bking"]->loadFromFile("resource/Bking.png"))
    {
        std::cout<<"Bad texture path.\n";
        exit(-1);
    }
    textures["Wknight"] = new sf::Texture();
    if (!textures["Wknight"]->loadFromFile("resource/Wknight.png"))
    {
        std::cout<<"Bad texture path.\n";
        exit(-1);
    }
    textures["Bknight"] = new sf::Texture();
    if (!textures["Bknight"]->loadFromFile("resource/Bknight.png"))
    {
        std::cout<<"Bad texture path.\n";
        exit(-1);
    }
    textures["Wbishop"] = new sf::Texture();
    if (!textures["Wbishop"]->loadFromFile("resource/Wbishop.png"))
    {
        std::cout<<"Bad texture path.\n";
        exit(-1);
    }
    textures["Bbishop"] = new sf::Texture();
    if (!textures["Bbishop"]->loadFromFile("resource/Bbishop.png"))
    {
        std::cout<<"Bad texture path.\n";
        exit(-1);
    }
    textures["Wrook"] = new sf::Texture();
    if (!textures["Wrook"]->loadFromFile("resource/Wrook.png"))
    {
        std::cout<<"Bad texture path.\n";
        exit(-1);
    }
    textures["Brook"] = new sf::Texture();
    if (!textures["Brook"]->loadFromFile("resource/Brook.png"))
    {
        std::cout<<"Bad texture path.\n";
        exit(-1);
    }
    textures["Wqueen"] = new sf::Texture();
    if (!textures["Wqueen"]->loadFromFile("resource/Wqueen.png"))
    {
        std::cout<<"Bad texture path.\n";
        exit(-1);
    }
    textures["Bqueen"] = new sf::Texture();
    if (!textures["Bqueen"]->loadFromFile("resource/Bqueen.png"))
    {
        std::cout<<"Bad texture path.\n";
        exit(-1);
    }
    textures["Wpawn"] = new sf::Texture();
    if (!textures["Wpawn"]->loadFromFile("resource/Wpawn.png"))
    {
        std::cout<<"Bad texture path.\n";
        exit(-1);
    }
    textures["Bpawn"] = new sf::Texture();
    if (!textures["Bpawn"]->loadFromFile("resource/Bpawn.png"))
    {
        std::cout<<"Bad texture path.\n";
        exit(-1);
    }
}

bool Chessboard::handlePawnMoves(Piece* pawn, const sf::Vector2i& mv, const Vector2i& targetPos)
{
    Piece* onTargetSquare = config.getPiece(targetPos);
    bool result = false;
    bool promotionFlag = targetPos.y == 0 || targetPos.y == 7 ? true : false;
    if (mv.x == 0 && onTargetSquare == nullptr)
    {
        if (!promotionFlag)
            config.movePiece(pawn->getID(), targetPos);
        result = true;
    }
    else if (mv.x == 0)
        return false;
    if (onTargetSquare != nullptr && onTargetSquare->getColor() != pawn->getColor())
    {
        if (!promotionFlag)
            config.movePiece(pawn->getID(), targetPos);
        graphics->disableSprite(onTargetSquare->getID());
        result = true;
    }
    else if (config.isEnPassantPossible(pawn) && mv == config.getEnPassantVector(pawn))
    {
        int capturedPawnID = config.enPassant(pawn);
        graphics->disableSprite(capturedPawnID);
        result = true;
    }
    if (targetPos.y == 0 || targetPos.y == 7)
    {
        graphics->setPromotionFlag(pawn->getID());
        promotionPosition = targetPos;
    }
    return result;
}

bool Chessboard::handleKingMoves(Piece* king, const sf::Vector2i& mv, const Vector2i& targetPos)
{
    int rookID;
    if (mv.x == 2 || mv.x == -2)
    {
        if ((rookID = config.isCastlingAvailable(king, mv)) == 0)
            return false;
        Vector2i kingPos = king->getPos();
        config.castle(king, mv);
        graphics->moveSprite(rookID, Vector2i(kingPos.x + mv.x / 2, kingPos.y));
        return true;
    }
    if (config.isChecked(targetPos, ++king->getColor()) != nullptr)
        return false;
    return handleOtherMoves(king, mv, targetPos);
}

bool Chessboard::handleOtherMoves(Piece* movingPiece, const sf::Vector2i& mv, const Vector2i& targetPos)
{
    Piece* onTargetSquare = config.getPiece(targetPos);
    if (onTargetSquare == nullptr)
    {
        config.movePiece(movingPiece->getID(), targetPos);
        return true;
    }
    else if (movingPiece->getColor() != onTargetSquare->getColor())
    {
        config.movePiece(movingPiece->getID(), targetPos);
        graphics->disableSprite(onTargetSquare->getID());
        return true;
    }
    return false;
}

bool Chessboard::handlePromotion()
{
    PromotionChoice choice = graphics->updatePromotionBar(window);
    PieceType promoteTo = PieceType::PAWN;
    switch (choice)
    {
    case PromotionChoice::QUEEN:
        promoteTo = PieceType::QUEEN;
        break;
    case PromotionChoice::ROOK:
        promoteTo = PieceType::ROOK;
        break;
    case PromotionChoice::BISHOP:
        promoteTo = PieceType::BISHOP;
        break;
    case PromotionChoice::KNIGHT:
        promoteTo = PieceType::KNIGHT;
        break;
    case PromotionChoice::NONE:
        return false;
    }
    config.movePiece(graphics->getPromotionFlag(), promotionPosition, promoteTo);
    graphics->loadFromConfig(&config);
    graphics->setPromotionFlag(-1);
    testDynamics();
    return true;
}

void Chessboard::updateNavbar()
{
    if (!engine->isMate() && !engine->isStealmate())
    {
        for (int i = 0; i < 3; i++)
        {
            Move2 move = engine->getRandomMove();
            Move moveExtended(move.pieceType, config.getSideOnMove(), move.pieceID, move.targetPos, config.getMoveCount(),
                              move.promotionFlag, move.specialFlag, 0.f);
            navbar->setMove(moveExtended, i);
        }
    }
}

Chessboard::Chessboard(sf::RenderWindow* win, float tileSize) : config()
{
    window = win;
    initTextures();
    graphics = new ChessboardUI(this, tileSize, textures);
    navbar = new Navbar(sf::Vector2f(800.f, 0.f),  sf::Vector2f(200.f, 800.f), textures);
    graphics->loadFromConfig(&config);
    navbar->setMove(Move(PieceType::PAWN, COLOR::WHITE, 5, sf::Vector2i(4, 4), 1,
                         PieceType::PAWN, MoveType::COMMON, 0.4f), 0);
    navbar->setMove(Move(PieceType::PAWN, COLOR::WHITE, 4, sf::Vector2i(3, 4), 1,
                         PieceType::PAWN, MoveType::COMMON, 0.3f), 1);
    navbar->setMove(Move(PieceType::KNIGHT, COLOR::WHITE, 10, sf::Vector2i(5, 5), 1,
                         PieceType::PAWN, MoveType::COMMON, 0.3f), 2);
    std::string FEN = "6k1/6r1/8/8/8/8/6B1/6KQ w - - 0 1";
    config.setFromFEN(FEN);
    graphics->loadFromConfig(&config);
    engine = new Engine(&config);
    engine->setupPosition(FEN);
    engine->showPositionStats();
}

Chessboard::~Chessboard()
{
    delete graphics;
    delete engine;
}

bool Chessboard::registerMovement(const Vector2i& initSquare, const Vector2i& targetSquare)
{
    if (targetSquare == initSquare)
        return false;
    Piece* movingPiece = config.getPiece(initSquare);
    if (movingPiece == nullptr)
        throw std::bad_exception();
    Vector2i movementVector = movingPiece->getMovementVector(targetSquare);
    if (movementVector.x >= 8 || config.checkCollisions(initSquare, movementVector, targetSquare) != nullptr)
        return false;
    if (!config.isCoveringChecks(movingPiece, targetSquare))
        return false;
    bool result = false;
    if (BoardConfig::isKing(movingPiece))
        result = handleKingMoves(movingPiece, movementVector, targetSquare);
    else
    {
        Vector2i pinMark = config.isPinned(movingPiece);
        if (pinMark.x < 8 && pinMark != movementVector && pinMark != -movementVector)
            return false;
        if (BoardConfig::isPawn(movingPiece))
            result = handlePawnMoves(movingPiece, movementVector, targetSquare);
        else
            result = handleOtherMoves(movingPiece, movementVector, targetSquare);
    }
    if (result)
        updateNavbar();
    return result;
}

void Chessboard::updateBoard()
{
    if (graphics->getPromotionFlag() > 0)
    {
        handlePromotion();
        return;
    }
    graphics->updateSprites(window);
    Navbar::NavbarState action = navbar->update(window);
    switch (action.type)
    {
    case ButtonType::RESET:
        if (!clickState)
        {
            config.setToDefault();
            graphics->loadFromConfig(&config);
            clickState = true;
            updateNavbar();
        }
        break;
    case ButtonType::BACK:
        if (!clickState && config.undoLastMove())
        {
            graphics->loadFromConfig(&config);
            testDynamics();
            clickState = true;
            updateNavbar();
        }
        break;
    case ButtonType::ENTRYMARKED:
        if (!graphics->getArrowState())
        {
            Move moveInfo = navbar->getMove(action.entryID);
            graphics->setArrow(moveInfo.pieceID, moveInfo.targetPos);
        }
        break;
    default:
        graphics->disableArrow();
        clickState = false;
        break;
    }
}

void Chessboard::render(sf::RenderTarget& target)
{
    graphics->draw(target, sf::RenderStates());
    navbar->render(target);
}



void Chessboard::testDynamics() const
{
    system("cls");
    engine->showPositionStats();
    std::cout << std::endl;
}

