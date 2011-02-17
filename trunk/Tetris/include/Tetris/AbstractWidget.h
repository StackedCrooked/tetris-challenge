#ifndef TETRIS_ABSTRACTWIDGET_H_INCLUDED
#define TETRIS_ABSTRACTWIDGET_H_INCLUDED


#include "Tetris/BlockType.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/Grid.h"
#include "Tetris/Player.h"
#include "Tetris/SimpleGame.h"


namespace Tetris {


class SimpleGame;


class RGBColor
{
public:
    RGBColor(int red, int green, int blue);

    int red() const;

    int green() const;

    int blue() const;

private:
    int mRed, mGreen, mBlue;
};


class Rect
{
public:
    Rect(int x, int y, int width, int height);

    inline int x() const { return mX; }

    inline int y() const { return mY; }

    inline int width() const { return mWidth; }

    inline int height() const { return mHeight; }

    inline int left() const { return mX; }

    inline int right() const { return mX + mWidth; }

    inline int top() const { return mY; }

    inline int bottom() const { return mY + mHeight; }

private:
    int mX, mY, mWidth, mHeight;
};


class Size
{
public:
    Size(int width, int height);

    inline int width() const { return mWidth; }

    inline int height() const { return mHeight; }

private:
    int mWidth, mHeight;
};


class AbstractWidget : public SimpleGame::EventHandler,
                       public SimpleGame::BackReference
{
public:
    AbstractWidget(int insquareWidth, int inSquareHeight);

    virtual ~AbstractWidget();

    virtual void onGameStateChanged(SimpleGame * inGame);

    virtual void onLinesCleared(SimpleGame * inGame, int inLineCount);

    virtual void onDestroy(SimpleGame * inGame);

    void setPlayer(Player * inPlayer);

    const SimpleGame * simpleGame() const;

    SimpleGame * simpleGame();

    const Player * player() const;

    Player * player();

    int fps() const;

    int squareWidth() const;

    int squareHeight() const;

    int margin() const;

    int statsItemHeight() const;

    int statsItemCount() const;

    virtual const RGBColor & getColor(BlockType inBlockType) const;

    virtual void refresh() = 0;

    virtual Rect captionRect() const;

    virtual Rect gameRect() const;

    virtual Rect statsRect() const;

    virtual Rect futureBlocksRect() const;

protected:
    virtual void setMinSize(const Size & inMinSize) = 0;
    virtual Size getMinSize() const = 0;

    void coordinateRepaint(const SimpleGame & inGame);
    virtual void paintSquare(const Rect & inRect, const RGBColor & inColor) = 0;
    virtual void paintStatItem(const Tetris::Rect & inRect, const std::string & inName, const std::string & inValue) = 0;
    virtual void drawRect(const Rect & inRect, const RGBColor & inColor) = 0;
    virtual void fillRect(const Rect & inRect, const RGBColor & inColor) = 0;
    virtual void drawLine(int x1, int y1, int x2, int y2, int inPenWidth, const RGBColor & inColor) = 0;
    virtual void drawText(int x, int y, const std::string & inText) = 0;

private:
    void paintGrid(int x, int y, const Grid & inGrid);
    void paintGrid(int x, int y, const Grid & inGrid, const RGBColor & inColor);
    void paintCaption();
    void paintGameGrid(const Grid & inGrid);
    void paintActiveBlockShadow(const SimpleGame & inSimpleGame);
    void paintStats(const Rect & inRect, const GameStateStats & inStats);
    void paintFutureBlocks(const Rect & inRect, int inSpacing, const std::vector<BlockType> & inBlockTypes);
    void recalculateFPS();

    Tetris::Player * mPlayer;
    int mSquareWidth;
    int mSquareHeight;
    int mStatItemHeight;
    int mCaptionRectHeight;
    int mSpacing;
    int mMargin;
    bool mPaintActiveBlockShadow;
    unsigned int mFrameCount;
    double mFPS;
};


} // namespace Tetris


#endif // TETRIS_ABSTRACTWIDGET_H_INCLUDED
