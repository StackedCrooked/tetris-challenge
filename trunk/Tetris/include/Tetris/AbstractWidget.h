#ifndef TETRIS_ABSTRACTWIDGET_H_INCLUDED
#define TETRIS_ABSTRACTWIDGET_H_INCLUDED


#include "Tetris/BlockType.h"
#include "Tetris/Grid.h"
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


class AbstractWidget : public SimpleGame::EventHandler
{
public:
    AbstractWidget(int insquareWidth, int inSquareHeight);

    virtual ~AbstractWidget();

    virtual void onSimpleGameChanged();

    void setGame(Tetris::SimpleGame * inSimpleGame);

    const Tetris::SimpleGame * getGame() const;

    Tetris::SimpleGame * getGame();

    int getFPS() const;

    int squareWidth() const;

    int squareHeight() const;

    virtual const RGBColor & getColor(BlockType inBlockType) const;

protected:
    virtual void scheduleRefresh() = 0;
    virtual void setMinSize(int inWidth, int inHeight) = 0;
    void coordinateRepaint(const SimpleGame & inGame);
    virtual void paintSquare(const Rect & inRect, const RGBColor & inColor) = 0;
    virtual void drawLine(int x1, int y1, int x2, int y2, int inPenWidth, const RGBColor & inColor) = 0;
    virtual Rect getGameRect() const = 0;
    virtual Rect getFutureBlocksRect(unsigned int inFutureBlockCount) const = 0;

private:
    void paintGrid(int y, int x, const Grid & inGrid);
    void paintGameGrid(const Grid & inGrid);
    void paintFutureBlocks(const Rect & inRect, int inSpacing, const std::vector<BlockType> & inBlockTypes);
    void recalculateFPS();

    Tetris::SimpleGame * mSimpleGame;
    int mSquareWidth;
    int mSquareHeight;
    int mSpacing;
    int mMargin;
    unsigned int mFrameCount;
    double mFPS;
};


} // namespace Tetris


#endif // TETRIS_ABSTRACTWIDGET_H_INCLUDED
