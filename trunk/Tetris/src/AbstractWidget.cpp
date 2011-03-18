#include "Tetris/Config.h"
#include "Tetris/AbstractWidget.h"
#include "Tetris/Block.h"
#include "Tetris/Game.h"
#include "Tetris/MakeString.h"
#include "Tetris/SimpleGame.h"
#include "Tetris/Threading.h"
#include <boost/bind.hpp>


namespace Tetris {


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


Size::Size(int width, int height) :
    mWidth(width),
    mHeight(height)
{
}


AbstractWidget::AbstractWidget(int inSquareWidth, int inSquareHeight) :
    mPlayer(0),
    mSquareWidth(inSquareWidth),
    mSquareHeight(inSquareHeight),
    mStatItemHeight(45),
    mAvatarWidth(80),
    mSpacing(5),
    mMargin(2),
    mPaintActiveBlockShadow(false),
    mFrameCount(0),
    mFPS(0)
{
    mFPSStopwatch.start();
}


AbstractWidget::~AbstractWidget()
{
    if (mPlayer)
    {
        SimpleGame::UnregisterEventHandler(mPlayer->simpleGame(), this);
    }
}


void AbstractWidget::onGameStateChanged(SimpleGame * inSimpleGame)
{
    if (!mPlayer || mPlayer->simpleGame() != inSimpleGame)
    {
        throw std::logic_error("AbstractWidget::onDestroy: inGame != mGame");
    }

    refresh();
}


void AbstractWidget::onLinesCleared(SimpleGame * inSimpleGame, int )
{
    if (!mPlayer || mPlayer->simpleGame() != inSimpleGame)
    {
        throw std::logic_error("AbstractWidget::onDestroy: inGame != mGame");
    }

    refresh();
}


void AbstractWidget::onDestroy(SimpleGame * inSimpleGame)
{
    if (!mPlayer || mPlayer->simpleGame() != inSimpleGame)
    {
        throw std::logic_error("AbstractWidget::onDestroy: inGame != mGame");
    }

    SimpleGame::UnregisterEventHandler(mPlayer->simpleGame(), this);
    mPlayer = 0;
}


void AbstractWidget::setPlayer(Player * inPlayer)
{
    // Stop listing to the events from the old player.
    if (mPlayer && SimpleGame::Exists(mPlayer->simpleGame()))
    {
        SimpleGame::UnregisterEventHandler(mPlayer->simpleGame(), this);
    }

    // Set the new player.
    mPlayer = inPlayer;

    // Start listening to the events from the new player.
    if (mPlayer && SimpleGame::Exists(mPlayer->simpleGame()))
    {
        setMinSize(Size(2 * margin() + futureBlocksRect().right() - gameRect().left(),
                        2 * margin() + avatarRect().bottom() - gameRect().top()));

        SimpleGame::RegisterEventHandler(mPlayer->simpleGame(), this);
    }
}


const SimpleGame * AbstractWidget::simpleGame() const
{
    if (!mPlayer)
    {
        return NULL;
    }
    return mPlayer->simpleGame();
}


SimpleGame * AbstractWidget::simpleGame()
{
    if (!mPlayer)
    {
        return NULL;
    }
    return mPlayer->simpleGame();
}


const Player * AbstractWidget::player() const
{
    return mPlayer;
}


Player * AbstractWidget::player()
{
    return mPlayer;
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
    return 4; // Score, Level, Line and Fps.
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
    int x = margin();
    int y = margin();
    int w = simpleGame()->gameGrid().columnCount() * squareWidth();
    int h = simpleGame()->gameGrid().rowCount() * squareHeight();
    return Rect(x, y, w, h);
}


Rect AbstractWidget::userInfoRect() const
{
    Rect theGameRect = gameRect();
    int x = margin();
    int y = theGameRect.bottom() + margin();
    int width = theGameRect.width();
    int height = avatarRect().bottom() - y;
    return Rect(x, y, width, height);
}



Rect AbstractWidget::futureBlocksRect() const
{
    int numFutureBlocks = simpleGame()->futureBlocksCount();
    int blockHeight = 2 * squareHeight();
    int totalHeight = 2 * margin() + numFutureBlocks * blockHeight;
    if (numFutureBlocks > 1)
    {
        totalHeight += (numFutureBlocks - 1) * squareHeight();
    }
    Rect theGameRect = gameRect();
    int x = theGameRect.right() + margin();
    int y = theGameRect.top();
    int w = mAvatarWidth;
    int h = totalHeight;
    return Rect(x, y, w, h);
}


Rect AbstractWidget::statsRect() const
{
    Rect theFutureBlocksRect = futureBlocksRect();
    int x = theFutureBlocksRect.x();
    int y = theFutureBlocksRect.bottom() + margin();
    int w = theFutureBlocksRect.width();
    int h = statsItemCount() * statsItemHeight();
    if (statsItemCount() > 1)
    {
        h += (statsItemCount() - 1) * margin();
    }
    return Rect(x, y, w, h);
}


Rect AbstractWidget::avatarRect() const
{
    Rect theStatsRect = statsRect();
    int x = theStatsRect.x();
    int y = theStatsRect.bottom() + margin();
    int width = mAvatarWidth;
    int height = mAvatarWidth;
    return Rect(x, y, width, height);
}


void AbstractWidget::coordinateRepaint(SimpleGame & inGame)
{
    Tetris::Size minSize = getMinSize();
    fillRect(Rect(0, 0, minSize.width(), minSize.height()), RGBColor(0, 50, 100));


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
    Rect theGameRect(gameRect());
    paintGrid(theGameRect.left() + activeBlock.column() * mSquareWidth,
              theGameRect.top() + activeBlock.row() * mSquareHeight,
              activeBlock.grid());

    // Paint shadow
    if (mPaintActiveBlockShadow)
    {
        paintActiveBlockShadow(inGame);
    }

    paintAvatar(inGame);

    paintUserInfo();

    recalculateFPS();
}


void AbstractWidget::paintGrid(int x, int y, const Grid & inGrid)
{
    for (size_t c = 0; c < inGrid.columnCount(); ++c)
    {
        for (size_t r = 0; r < inGrid.rowCount(); ++r)
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


void AbstractWidget::paintGrid(int x, int y, const Grid & inGrid, const RGBColor & inColor)
{
    for (size_t c = 0; c < inGrid.columnCount(); ++c)
    {
        for (size_t r = 0; r < inGrid.rowCount(); ++r)
        {
            BlockType blockType = inGrid.get(r, c);
            if (blockType != BlockType_Nil)
            {
                paintSquare(Rect(x + (c * mSquareWidth),
                                 y + (r * mSquareHeight),
                                 mSquareWidth,
                                 mSquareHeight),
                            inColor);
            }
        }
    }
}


void AbstractWidget::paintUserInfo()
{
    Rect theuserInfoRect = userInfoRect();
    fillRect(theuserInfoRect, RGBColor(255, 255, 255));
    drawTextRightAligned(theuserInfoRect, player()->teamName() + ": " + player()->playerName(), 14, RGBColor(0, 255, 255));
}


void AbstractWidget::paintGameGrid(const Grid & inGrid)
{
    Rect theGameRect = gameRect();
    fillRect(theGameRect, RGBColor(255, 255, 255));
    paintGrid(theGameRect.x(), theGameRect.y(), inGrid);
}


void AbstractWidget::paintAvatar(const SimpleGame & inSimpleGame)
{
    Rect theAvatarRect = avatarRect();
    paintImage(theAvatarRect, player()->playerName() + "_80.gif");
}


void AbstractWidget::paintActiveBlockShadow(const SimpleGame & inSimpleGame)
{
    size_t colIdx(0);
    size_t rowIdx(0);
    boost::scoped_ptr<Grid> gridPtr;

    // Critical section. Minimize scope.
    {
        ScopedReader<Game> rgame(inSimpleGame.game());
        const Game & game = *rgame.get();
        const Block & block = game.activeBlock();
        const GameState & gameState = game.gameState();

        colIdx = block.column();
        gridPtr.reset(new Grid(block.grid()));
        for (; rowIdx < game.rowCount(); ++rowIdx)
        {
            if (!gameState.checkPositionValid(block, rowIdx, colIdx))
            {
                if (rowIdx == 0)
                {
                    return; // This game is over.
                }

                rowIdx--;
                break;
            }
        }
    }


    Grid & grid = *gridPtr;

    Rect theGameRect(gameRect());

    int x = theGameRect.left() + colIdx * mSquareWidth;
    int y = theGameRect.top() + rowIdx * mSquareHeight;

    int gray = 240;
    paintGrid(x, y, grid, RGBColor(gray, gray, gray));
}


void AbstractWidget::paintStats(const Rect & inRect, const GameStateStats & inStats)
{
    Rect rect = Rect(inRect.x(), inRect.y(), inRect.width(), mStatItemHeight);
    fillRect(rect, RGBColor(255, 255, 255));
    paintStatItem(rect, "Score", MakeString() << inStats.score());

    rect = Rect(rect.left(), rect.bottom() + margin(), rect.width(), rect.height());
    fillRect(rect, RGBColor(255, 255, 255));
    paintStatItem(rect, "Level", MakeString() << simpleGame()->level());

    rect = Rect(rect.left(), rect.bottom() + margin(), rect.width(), rect.height());
    fillRect(rect, RGBColor(255, 255, 255));
    paintStatItem(rect, "Lines", MakeString() << inStats.numLines());

    rect = Rect(rect.left(), rect.bottom() + margin(), rect.width(), rect.height());
    fillRect(rect, RGBColor(255, 255, 255));

    paintStatItem(rect, "Fps", MakeString() << static_cast<int>(0.5 + mFPS));
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

    if (mFPSStopwatch.elapsed() > 1 * 1000.0 * 1000.0)
    {
        mFPS = (1000.0 * 1000.0 * mFrameCount) / static_cast<double>(mFPSStopwatch.elapsed());
        mFrameCount = 0;
        mFPSStopwatch.restart();
    }
}


} // namespace Tetris
