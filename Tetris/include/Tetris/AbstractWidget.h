#ifndef TETRIS_ABSTRACTWIDGET_H_INCLUDED
#define TETRIS_ABSTRACTWIDGET_H_INCLUDED


#include "Tetris/BlockType.h"
#include "Tetris/Grid.h"


namespace Tetris {


class Game;


class RGBColor
{
public:
    RGBColor(int red, int green, int blue);

    inline int red() const { return mRed; }

    inline int green() const { return mGreen; }

    inline int blue() const { return mBlue; }

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

private:
    int mX, mY, mWidth, mHeight;
};


class AbstractWidget
{
public:
    AbstractWidget(int inUnitWidth, int inUnitHeight);

    inline int unitWidth() const { return mUnitWidth; }

    inline int unitHeight() const { return mUnitHeight; }

    // The subclass may call this method in order to have the block repainted.
    void coordinateRepaint(const Game & inGame);

    // This method returns the official color for each block.
    // You are free to use your own colors however.
    const RGBColor & getColor(BlockType inBlockType) const;

    //
    // These methods must be implemented by the subclass.
    //
    virtual void paintRect(const Rect & inRect, const RGBColor & inColor) = 0;

    virtual void drawLine(int x1, int y1, int x2, int y2, int inPenWidth, const RGBColor & inColor) = 0;

    virtual Rect getGameRect() const = 0;

    virtual Rect getFutureBlocksRect(unsigned int inFutureBlockCount) const = 0;

private:
    void paintGrid(int y, int x, const Grid & inGrid);
    void paintGameGrid(const Grid & inGrid);
    void paintFutureBlocks(const Rect & inRect, int inSpacing, const std::vector<BlockType> & inBlockTypes);

    int mUnitWidth;
    int mUnitHeight;
};


} // namespace Tetris


#endif // TETRIS_ABSTRACTWIDGET_H_INCLUDED
