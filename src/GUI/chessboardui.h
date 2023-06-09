#ifndef CHESSBOARDUI_H
#define CHESSBOARDUI_H

#include <vector>
#include <map>
#include <string>
#include "bluredcircle.h"
#include "promotionbar.h"
#include "roundedrectangle.h"
#include "arrow.h"

class Chessboard;
class BoardConfig;

class ChessboardUI : public sf::Drawable, public sf::Transformable
{
private:
    // Communitacion with logic board
    Chessboard* logicBoard;
    // GUI parameters
    float tileSize;
    PieceImage* selectedPiece = nullptr;
    bool mouseState = false;
    bool redCircleState = false;
    bool moveRectanglesState = false;
    bool arrowState = false;
    int colorID = 0;
    int awaitingPromotion = -1;
    static constexpr int circleCount = 30;
    // Graphics
    sf::VertexArray vertices;
    std::vector<PieceImage> pieces;
    std::map<std::string, sf::Texture*>& textures;
    sf::Texture squaresTexture;
    BluredCircle redCircle;
    sf::RectangleShape movedTo;
    sf::RectangleShape movedFrom;
    sf::RectangleShape fog;
    PromotionBar whitePromotionBar;
    PromotionBar blackPromotionBar;
    PromotionBar* currentPromotionBar;
    MoveArrow arrow;
    // Private functions
    void loadSquares();
    void initTextures();
    void initSprites();
    void setTextureFor(PieceImage& piece, COLOR color, PieceType type);
    void updateRedCircle(BoardConfig* config);
    void placePromotionBar();
public:
    ChessboardUI(Chessboard* logicBoard, float tsize, std::map<std::string, sf::Texture*>& txts);
    virtual ~ChessboardUI();
    void moveSprite(int spriteID, const sf::Vector2i& pos);
    void disableSprite(int spriteID) {pieces[spriteID].setState(false);}
    void loadFromConfig(BoardConfig* config);
    void setPromotionFlag(int promotedPieceID);
    int getPromotionFlag() const {return awaitingPromotion;}
    bool getArrowState() const {return arrowState;}
    void setArrow(int pieceID, const sf::Vector2i& targetSquare);
    void disableArrow() {arrowState = false;}
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
    void updateSprites(sf::RenderWindow* window);
    PromotionChoice updatePromotionBar(sf::RenderWindow* window);
};

#endif // CHESSBOARDUI_H
