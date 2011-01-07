#include "Tetris/Config.h"
#include "Tetris/AbstractWidget.h"
#include "Tetris/Block.h"
#include "Tetris/Game.h"
#include "Poco/Stopwatch.h"


namespace Tetris {


static const int cSpacing = 5;
static const int cFutureBlockCount = 3;



static Poco::Stopwatch sFPSStopwatch;


RGBColor::RGBColor(int red, int green, int blue) :
    mRed(red),
    mGreen(green),
    mBlue(blue)
{
}


Rect::Rect(int x, int y, int width, int height) :
    mX(x),
    mY(y),
    mWidth(width),
    mHeight(height)
{
}


AbstractWidget::AbstractWidget(int inSquareWidth, int inSquareHeight) :
    mSquareWidth(inSquareWidth),
    mSquareHeight(inSquareHeight),
    mFrameCount(0),
    mFPS(0)
{
    sFPSStopwatch.start();
}


const RGBColor & AbstractWidget::getColor(BlockType inBlockType) const
{
    static const RGBColor fColors[] =
    {
        RGBColor(255, 255, 255),      // Background
        RGBColor(  0, 255, 255),      // I-Shape
        RGBColor(  0, 0,   255),      // J-Shape
        RGBColor(255, 165,   0),      // L-Shape
        RGBColor(255, 255,   0),      // O-Shape
        RGBColor(  0, 255,   0),      // S-Shape
        RGBColor(160,  32, 240),      // T-Shape
        RGBColor(255,   0,   0)       // Z-Shape
    };
    return fColors[static_cast<int>(inBlockType)];
}


void AbstractWidget::coordinateRepaint(const Game & inGame)
{
    // Get the rects
    Rect gameRect(getGameRect());
    std::vector<BlockType> futureBlocks;
    inGame.getFutureBlocksWithOffset(inGame.currentBlockIndex() + 1, cFutureBlockCount, futureBlocks);
    Rect futureBlocksRect(getFutureBlocksRect(futureBlocks.size()));

    // Clear the rects
    paintSquare(gameRect, RGBColor(255, 255, 255));
    paintSquare(futureBlocksRect, RGBColor(255, 255, 255));

    // Paint the game
    paintGameGrid(inGame.gameGrid());

    // Paint future blocks
    paintFutureBlocks(futureBlocksRect, cSpacing, futureBlocks);


    // Paint active block
    const Block & activeBlock(inGame.activeBlock());
    paintGrid(activeBlock.column() * mSquareWidth,
              activeBlock.row() * mSquareHeight,
              activeBlock.grid());

    mFrameCount++;

    if (sFPSStopwatch.elapsed() > 1000000)
    {
        mFPS = (1000.0 * 1000.0 * mFrameCount) / static_cast<double>(sFPSStopwatch.elapsed());
        mFrameCount = 0;
        sFPSStopwatch.restart();
    }
}


void AbstractWidget::paintGrid(int x, int y, const Grid & inGrid)
{
    for (unsigned int c = 0; c < inGrid.columnCount(); ++c)
    {
        for (unsigned int r = 0; r < inGrid.rowCount(); ++r)
        {
            BlockType blockType = inGrid.get(r, c);
            if (blockType != BlockType_Nil)
            {
                paintSquare(Rect(x + (c * mSquareWidth),
                               y + (r * mSquareHeight),
                               mSquareWidth,
                               mSquareHeight),
                          getColor(blockType));
            }
        }
    }
}


void AbstractWidget::paintGameGrid(const Grid & inGrid)
{
    Rect gameRect = getGameRect();
    paintGrid(gameRect.x(), gameRect.y(), inGrid);
}


void AbstractWidget::paintFutureBlocks(const Rect & inRect, int inSpacing, const std::vector<BlockType> & inBlockTypes)
{
    if (inBlockTypes.empty())
    {
        return;
    }

    int y = inRect.y();
    for (unsigned int i = 0; i < inBlockTypes.size(); ++i)
    {
        const Grid & grid(GetGrid(GetBlockIdentifier(inBlockTypes[i], 0)));
        paintGrid(inRect.x(), y, grid);
        y += 3 * squareHeight();
    }
}


} // namespace Tetris
