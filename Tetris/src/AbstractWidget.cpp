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
    mStatItemHeight(45),
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
        setMinSize(margin() + gameRect().width() + margin() + futureBlocksRect().width() + margin(),
                   margin() + gameRect().height() + margin());

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


int AbstractWidget::margin() const
{
    return mMargin;
}


int AbstractWidget::statsItemHeight() const
{
    return mStatItemHeight;
}


int AbstractWidget::statsItemCount() const
{
    return 3; // Score, Level and Lines.
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


Rect AbstractWidget::gameRect() const
{
    // This rect is relative to the Widget rect.
    return Rect(margin(),
                margin(),
                game()->gameGrid().columnCount() * squareWidth(),
                game()->gameGrid().rowCount() * squareHeight());
}


Rect AbstractWidget::futureBlocksRect() const
{
    int numFutureBlocks = game()->futureBlocksCount();
    int blockHeight = 2 * squareHeight();
    int totalHeight = 2 * margin() + numFutureBlocks * blockHeight;
    if (numFutureBlocks > 1)
    {
        totalHeight += (numFutureBlocks - 1) * squareHeight();
    }
    return Rect(gameRect().right() + margin(),
                margin(),
                4 * squareWidth() + 2 * margin(),
                totalHeight);
}


Rect AbstractWidget::statsRect() const
{
    int requiredHeight = statsItemCount() * statsItemHeight();
    if (statsItemCount() > 1)
    {
        requiredHeight += (statsItemCount() - 1) * margin();
    }

    Rect theGameRect(gameRect());
    Tetris::Rect theFutureBlocksRect = futureBlocksRect();

    int x = theFutureBlocksRect.left();
    int y = theGameRect.bottom() - requiredHeight;
    return Tetris::Rect(x, y, theFutureBlocksRect.width(), requiredHeight);
}


void AbstractWidget::coordinateRepaint(const SimpleGame & inGame)
{
    // Paint the game
    paintGameGrid(inGame.gameGrid());

    // Paint future blocks
    std::vector<Block> futureBlocks(inGame.getNextBlocks());
    std::vector<BlockType> blockTypes;
    for (std::vector<Block>::size_type idx = 0; idx < futureBlocks.size(); ++idx)
    {
        blockTypes.push_back(futureBlocks[idx].type());
    }
    paintFutureBlocks(futureBlocksRect(), mSpacing, blockTypes);

    // Paint the stats
    paintStats(statsRect(), inGame.stats());

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
    fillRect(theGameRect, RGBColor(255, 255, 255));
    paintGrid(theGameRect.x(), theGameRect.y(), inGrid);
}


void AbstractWidget::paintStats(const Rect & inRect, const GameStateStats & inStats)
{
    Rect rect = Rect(inRect.x(), inRect.y(), inRect.width(), mStatItemHeight);
    fillRect(rect, RGBColor(255, 255, 255));
    paintStatItem(rect, "Score", MakeString() << inStats.score());

    rect = Rect(rect.left(), rect.bottom() + margin(), rect.width(), rect.height());
    fillRect(rect, RGBColor(255, 255, 255));
    paintStatItem(rect, "Level", MakeString() << game()->level());

    rect = Rect(rect.left(), rect.bottom() + margin(), rect.width(), rect.height());
    fillRect(rect, RGBColor(255, 255, 255));
    paintStatItem(rect, "Lines", MakeString() << inStats.numLines());
}


void AbstractWidget::paintFutureBlocks(const Rect & inRect, int inSpacing, const BlockTypes & inBlockTypes)
{
    if (inBlockTypes.empty())
    {
        return;
    }

    fillRect(inRect, RGBColor(255, 255, 255));

    for (BlockTypes::size_type i = 0; i < inBlockTypes.size(); ++i)
    {
        const Grid & grid(GetGrid(GetBlockIdentifier(inBlockTypes[i], 0)));
        int x = inRect.left() + (inRect.width() - (grid.columnCount() * squareWidth()))/2;
        int y = margin() + inRect.top() + (i * 3 * squareHeight());
        paintGrid(x, y, grid);
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
