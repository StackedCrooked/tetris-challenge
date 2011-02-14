#include "Tetris/Config.h"
#include "Tetris/AbstractWidget.h"
#include "Tetris/Block.h"
#include "Tetris/MakeString.h"
#include "Tetris/SimpleGame.h"
#include "Poco/Stopwatch.h"
#include <boost/bind.hpp>


namespace Tetris {


static Poco::Stopwatch sFPSStopwatch;


RGBColor::RGBColor(int red, int green, int blue) :
    mRed(red),
    mGreen(green),
    mBlue(blue)
{
}


int RGBColor::red() const
{
    return mRed;
}


int RGBColor::green() const
{
    return mGreen;
}


int RGBColor::blue() const
{
    return mBlue;
}


Rect::Rect(int x, int y, int width, int height) :
    mX(x),
    mY(y),
    mWidth(width),
    mHeight(height)
{
}


AbstractWidget::AbstractWidget(int inSquareWidth, int inSquareHeight) :
    mSimpleGame(0),
    mSquareWidth(inSquareWidth),
    mSquareHeight(inSquareHeight),
    mSpacing(5),
    mMargin(4),
    mFrameCount(0),
    mFPS(0)
{
    sFPSStopwatch.start();
}


AbstractWidget::~AbstractWidget()
{
    SimpleGame::UnregisterEventHandler(mSimpleGame, this);
}


void AbstractWidget::onGameStateChanged(SimpleGame * inSimpleGame)
{
    if (inSimpleGame != mSimpleGame)
    {
        throw std::logic_error("AbstractWidget::onDestroy: inGame != mGame");
    }

    refresh();
}


void AbstractWidget::onLinesCleared(SimpleGame * inSimpleGame, int )
{
    if (inSimpleGame != mSimpleGame)
    {
        throw std::logic_error("AbstractWidget::onDestroy: inGame != mGame");
    }

    refresh();
}


void AbstractWidget::onDestroy(SimpleGame * inSimpleGame)
{
    if (inSimpleGame != mSimpleGame)
    {
        throw std::logic_error("AbstractWidget::onDestroy: inGame != mGame");
    }

    SimpleGame::UnregisterEventHandler(mSimpleGame, this);
    mSimpleGame = 0;
}


void AbstractWidget::setGame(SimpleGame * inSimpleGame)
{
    if (SimpleGame::Exists(mSimpleGame))
    {
        SimpleGame::UnregisterEventHandler(mSimpleGame, this);
        mSimpleGame->setBackReference(0);
    }

    mSimpleGame = inSimpleGame;
    if (SimpleGame::Exists(mSimpleGame))
    {
        setMinSize((mSimpleGame->columnCount() + 4) * squareWidth() + mMargin,
                   gameRect().height() + statsRect().height());

        SimpleGame::RegisterEventHandler(mSimpleGame, this);
        mSimpleGame->setBackReference(this);
    }
}


const Tetris::SimpleGame * AbstractWidget::game() const
{
    return mSimpleGame;
}


Tetris::SimpleGame * AbstractWidget::game()
{
    return mSimpleGame;
}


int AbstractWidget::fps() const
{
    return static_cast<int>(0.5 + mFPS);
}


int AbstractWidget::squareWidth() const
{
    return mSquareWidth;
}


int AbstractWidget::squareHeight() const
{
    return mSquareHeight;
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


void AbstractWidget::coordinateRepaint(const SimpleGame & inGame)
{
    // Get the rects
    Rect theGameRect(gameRect());
    std::vector<BlockType> futureBlocks;

    futureBlocks.push_back(inGame.getNextBlock().type());
    Rect theFutureBlocksRect(futureBlocksRect(futureBlocks.size()));

    // Clear the rects
    paintSquare(theGameRect, RGBColor(255, 255, 255));
    paintSquare(theFutureBlocksRect, RGBColor(255, 255, 255));

    // Paint the game
    paintGameGrid(inGame.gameGrid());

    // Paint future blocks
    paintFutureBlocks(theFutureBlocksRect, mSpacing, futureBlocks);

    GameStateStats stats = inGame.stats();

    Rect theStatsRect(statsRect());
    paintStats(theStatsRect, stats);


    // Paint active block
    const Block & activeBlock(inGame.activeBlock());
    paintGrid(activeBlock.column() * mSquareWidth,
              activeBlock.row() * mSquareHeight,
              activeBlock.grid());

    recalculateFPS();
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
    Rect theGameRect = gameRect();
    paintGrid(theGameRect.x(), theGameRect.y(), inGrid);
}


void AbstractWidget::paintStats(const Rect & inRect, const GameStateStats & inStats)
{
    int x = inRect.left();
    int y = inRect.top();

    drawText(x, y, MakeString() << "Total lines: " << inStats.numLines());
    y += squareHeight();

    drawText(x, y, MakeString() << "Lines x1: " << inStats.numSingles());
    y += squareHeight();

    drawText(x, y, MakeString() << "Lines x2: " << inStats.numDoubles());
    y += squareHeight();

    drawText(x, y, MakeString() << "Lines x3: " << inStats.numTriples());
    y += squareHeight();

    drawText(x, y, MakeString() << "Lines x4: " << inStats.numTetrises());
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


void AbstractWidget::recalculateFPS()
{
    mFrameCount++;

    if (sFPSStopwatch.elapsed() > 2 * 1000.0 * 1000.0)
    {
        mFPS = (1000.0 * 1000.0 * mFrameCount) / static_cast<double>(sFPSStopwatch.elapsed());
        mFrameCount = 0;
        sFPSStopwatch.restart();
    }
}


} // namespace Tetris
