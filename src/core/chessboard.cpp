#include <exception>
#include <iostream>
#include "chessboard.h"


bool Chessboard::handlePawnMoves(Piece* pawn, const Square& mv, const Square& targetPos)
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

bool Chessboard::handleKingMoves(Piece* king, const Square& mv, const Square& targetPos)
{
    int rookID;
    if (mv.x == 2 || mv.x == -2)
    {
        if ((rookID = config.isCastlingAvailable(king, mv)) == 0)
            return false;
        Square kingPos = king->getPos();
        config.castle(king, mv);
        graphics->moveSprite(rookID, Square(kingPos.x + mv.x / 2, kingPos.y));
        return true;
    }
    if (config.isChecked(targetPos, opposition(king->getColor())) != nullptr)
        return false;
    return handleOtherMoves(king, mv, targetPos);
}

bool Chessboard::handleOtherMoves(Piece* movingPiece, const Square& mv, const Square& targetPos)
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
        promoteTo = QUEEN;
        break;
    case PromotionChoice::ROOK:
        promoteTo = ROOK;
        break;
    case PromotionChoice::BISHOP:
        promoteTo = BISHOP;
        break;
    case PromotionChoice::KNIGHT:
        promoteTo = KNIGHT;
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
    /*if (!engine->isMate() && !engine->isStealmate())
    {
        for (int i = 0; i < 3; i++)
        {
            Move2 move = engine->getRandomMove();
            Move moveExtended(move.pieceType, config.getSideOnMove(), move.pieceID, move.targetPos, config.getMoveCount(),
                              move.promotionFlag, move.specialFlag, 0.f);
            navbar->setMove(moveExtended, i);
        }
    }*/
}

Chessboard::Chessboard(sf::RenderWindow* win, float tileSize) : config()
{
    window = win;
    graphics = new ChessboardUI(this, tileSize);
    navbar = new Navbar(sf::Vector2f(800.f, 0.f),  sf::Vector2f(200.f, 800.f));
    graphics->loadFromConfig(&config);
    navbar->setMove(Move(PieceType::PAWN, WHITE, 5, Square(4, 4), 1,
                         PieceType::PAWN, MoveType::COMMON, 0.4f), 0);
    navbar->setMove(Move(PieceType::PAWN, WHITE, 4, Square(3, 4), 1,
                         PieceType::PAWN, MoveType::COMMON, 0.3f), 1);
    navbar->setMove(Move(KNIGHT, WHITE, 10, Square(5, 5), 1,
                         PieceType::PAWN, MoveType::COMMON, 0.3f), 2);
    std::string FEN = "5qk1/1pp1prp1/p7/8/8/P1PP4/1PR3PP/1Q4K1 w - - 0 1";
    config.setFromFEN(FEN);
    graphics->loadFromConfig(&config);
    FactorsMap factors = FactorsDelivery::getFactors();
    engine = new Engine(&config, factors);
    config.connectEngine(engine);
    engine->showPositionStats();
}

Chessboard::~Chessboard()
{
    delete graphics;
    delete engine;
}

bool Chessboard::registerMovement(const Square& initSquare, const Square& targetSquare)
{
    if (targetSquare == initSquare)
        return false;
    Piece* movingPiece = config.getPiece(initSquare);
    if (movingPiece == nullptr)
        throw std::bad_exception();
    Square movementVector = movingPiece->getMovementVector(targetSquare);
    if (movementVector.x >= 8 || config.checkCollisions(initSquare, movementVector, targetSquare) != nullptr)
        return false;
    if (!config.isCoveringChecks(movingPiece, targetSquare))
        return false;
    bool result = false;
    if (BoardConfig::isKing(movingPiece))
        result = handleKingMoves(movingPiece, movementVector, targetSquare);
    else
    {
        Square pinMark = config.isPinned(movingPiece);
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
    //int eval = engine->evaluate();
    //std::cout << std::endl << "Global evaluation: "<<eval<<std::endl;
}

