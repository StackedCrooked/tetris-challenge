#include "TetrisWidget.h"
#include "Tetris/SimpleGame.h"
#include <QColor>
#include <QTimer>
#include <stdexcept>
#include <iostream>


// Margin between the game rect and the future blocks rect.
static const int cMargin = 4;


using namespace Tetris;


TetrisWidget::TetrisWidget(QWidget * inParent, int inUnitWidth, int inUnitHeight) :
    QWidget(inParent),
    AbstractWidget(inUnitWidth, inUnitHeight),
    mSimpleGame(0),
    mRowCount(20),
    mColCount(10),
    mMinSize(14 * inUnitWidth, 20 * inUnitHeight),
    mPainter()
{
    setUpdatesEnabled(true);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(30);

}


TetrisWidget::~TetrisWidget()
{
}


void TetrisWidget::setSimpleGame(SimpleGame * inSimpleGame)
{
    mSimpleGame = inSimpleGame;
    mSimpleGame->getSize(mColCount, mRowCount);
    mMinSize = QSize((mColCount + 4) * unitWidth(),
                     mRowCount * unitHeight());
}


void TetrisWidget::paintRect(const Rect & inRect, const RGBColor & inColor)
{
    if (!mPainter.get())
    {
        throw std::logic_error("Painter is not set.");
    }


    mPainter->fillRect(QRect(inRect.x(), inRect.y(), inRect.width(), inRect.height()),
                       QColor(inColor.red(), inColor.green(), inColor.blue()));
}


void TetrisWidget::drawLine(int x1, int y1, int x2, int y2, int inPenWidth, const RGBColor & inColor)
{
    if (!mPainter.get())
    {
        throw std::logic_error("Painter is not set.");
    }

    // todo: implement
}


Tetris::Rect TetrisWidget::getGameRect() const
{
    return Tetris::Rect(0,
                        0,
                        mColCount * unitWidth(),
                        mRowCount * unitHeight());
}


Tetris::Rect TetrisWidget::getFutureBlocksRect(unsigned int inFutureBlockCount) const
{
    int blockHeight = 4 * unitWidth();
    return Tetris::Rect(mColCount * unitHeight() + cMargin,
                        0,
                        blockHeight,
                        inFutureBlockCount * blockHeight);
}


void TetrisWidget::paintEvent(QPaintEvent * )
{
    if (!mSimpleGame)
    {
        return;
    }

    mPainter.reset(new QPainter(this));
    ScopedReader<Game> gameReader(mSimpleGame->getGame());
    const Game & game(*gameReader.get());
    coordinateRepaint(game);
    mPainter.reset();
}


QSize TetrisWidget::minimumSizeHint() const
{
    return mMinSize;
}
