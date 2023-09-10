#include <exception>
#include <iostream>
#include "chessboard.h"


void Chessboard::handlePawnMoves(const Piece* pawn, const Move& move)
{
    if (move.hasProperty(Moves::ENPASSANT_FLAG))
    {
        int capturedPawnID = config.getPiece(move.targetPos.x, BoardManipulation::enPassantRows[pawn->getColor()])->getID();
        config.makeMove(move);
        graphics->disableSprite(capturedPawnID);
    }
    else if (move.targetPos.y == 0 || move.targetPos.y == 7) // promotion
    {
        graphics->setPromotionFlag(pawn->getID());
        promotionPosition = move.targetPos;
    }
    else
        handleNormalMoves(pawn, move);
}

void Chessboard::handleKingMoves(const Piece* king, const Move& move)
{
    if (move.hasProperty(Moves::CASTLE_FLAG))
    {
        CastleType castleType = move.targetPos.x > king->getPosition().x ? SHORT_CASTLE : LONG_CASTLE;
        int rookID = config.isCastlingAvailable(king->getColor(), castleType);
        config.makeMove(move);
        graphics->moveSprite(rookID, BoardManipulation::rookPositionsAfterCastling[king->getColor()][castleType]);
    }
    else
        handleNormalMoves(king, move);
}

void Chessboard::handleNormalMoves(const Piece* movingPiece, const Move& move)
{
    const Piece* onTargetSquare = config.getPiece(move.targetPos);
    config.makeMove(move);
    if (onTargetSquare != nullptr)
        graphics->disableSprite(onTargetSquare->getID());
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
    int pieceID = graphics->getPromotionFlag();
    Move move{ pieceID, PAWN, promotionPosition, 0b1000, promoteTo };
    config.makeMove(move);
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
            Move move = engine->getRandomMove();
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
    /*navbar->setMove(Move(PieceType::PAWN, WHITE, 5, Square(4, 4), 1,
                         PieceType::PAWN, MoveType::COMMON, 0.4f), 0);
    navbar->setMove(Move(PieceType::PAWN, WHITE, 4, Square(3, 4), 1,
                         PieceType::PAWN, MoveType::COMMON, 0.3f), 1);
    navbar->setMove(Move(KNIGHT, WHITE, 10, Square(5, 5), 1,
                         PieceType::PAWN, MoveType::COMMON, 0.3f), 2);*/
    //std::string FEN = "7r/3P1k2/4pBp1/1ppb1pP1/5R2/r1P3KP/p7/3R4 b - - 0 43";
    //config.setFromFEN(FEN);
    graphics->loadFromConfig(&config);
    FactorsMap factors = FactorsDelivery::getFactors();
    engine = new Engine(&config, factors);
    config.connectEngine(engine);
    //engine->showPositionStats();
    config.showCustomStats();
}

Chessboard::~Chessboard()
{
    delete graphics;
    delete engine;
}

bool Chessboard::registerMovement(const Square& initSquare, const Square& targetSquare)
{
    std::optional<Move> moveOpt = Moves::createMove(&config, initSquare, targetSquare);
    if (!moveOpt.has_value())
        return false;
    const Move& move = moveOpt.value();
    if (!config.isMoveLegal(move))
        return false;
    const Piece* movingPiece = config.getPiece(move.pieceID);
    switch (move.pieceType)
    {
    case PAWN:
        handlePawnMoves(movingPiece, move);
        break;
    case KING:
        handleKingMoves(movingPiece, move);
        break;
    default:
        handleNormalMoves(movingPiece, move);
        break;
    }
    return true;
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
            Move2 moveInfo = navbar->getMove(action.entryID);
            graphics->setArrow(moveInfo.move.pieceID, moveInfo.move.targetPos);
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
    //system("cls");
    //engine->showPositionStats();
    config.showCustomStats();
}

