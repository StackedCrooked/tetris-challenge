#include "TetrisWidget.h"
#include "Tetris/Game.h"
#include "Tetris/SimpleGame.h"
#include "Tetris/Threading.h"
#include "Tetris/AutoPtrSupport.h"
#include <QKeyEvent>
#include <QColor>
#include <QMutexLocker>
#include <QTimer>
#include <stdexcept>
#include <iostream>


// Margin between the game rect and the future blocks rect.
static const int cMargin = 4;


using namespace Tetris;


TetrisWidget::TetrisWidget(QWidget * inParent, int inSquareWidth, int inSquareHeight) :
    QWidget(inParent),
    AbstractWidget(inSquareWidth, inSquareHeight),
    mRowCount(20),
    mColCount(10),
    mMinSize(),
    mRefreshFlag(), // Default contructor is fine.
    mRefreshFlagMutex(),
    mPainter()
{
    setUpdatesEnabled(true);
    setFocusPolicy(Qt::StrongFocus);

    QTimer * timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkRefreshFlag()));
    timer->setInterval(20);
    timer->start();
}


TetrisWidget::~TetrisWidget()
{
}


void TetrisWidget::checkRefreshFlag()
{
    bool doRefresh = false;
    {
        QMutexLocker locker(&mRefreshFlagMutex);
        if (mRefreshFlag)
        {
            doRefresh = mRefreshFlag;
            mRefreshFlag = false;
        }
    }

    if (doRefresh)
    {
        update();
    }
}


void TetrisWidget::keyPressEvent(QKeyEvent * inEvent)
{
    if (!getGame() || getGame()->isGameOver())
    {
        QWidget::keyPressEvent(inEvent);
        return;
    }


    switch (inEvent->key())
    {
        case Qt::Key_Left:
        {
            getGame()->move(Direction_Left);
            break;
        }
        case Qt::Key_Right:
        {
            getGame()->move(Direction_Right);
            break;
        }
        case Qt::Key_Down:
        {
            getGame()->move(Direction_Down);
            break;
        }
        case Qt::Key_Up:
        {
            getGame()->rotate();
            break;
        }
        case Qt::Key_Space:
        {
            getGame()->drop();
            break;
        }
        default:
        {
            QWidget::keyPressEvent(inEvent);
            break;
        }
    }
}


void TetrisWidget::scheduleRefresh()
{
    QMutexLocker lock(&mRefreshFlagMutex);
    mRefreshFlag = true;
}


void TetrisWidget::setMinSize(int inWidth, int inHeight)
{
    mMinSize = QSize(inWidth, inHeight);
}


void TetrisWidget::paintSquare(const Rect & inRect, const RGBColor & inColor)
{
    if (!mPainter.get())
    {
        throw std::logic_error("Painter is not set.");
    }


    QColor color(inColor.red(), inColor.green(), inColor.blue());
    int x = inRect.x();
    int y = inRect.y();
    int width = inRect.width();
    int height = inRect.height();

    mPainter->fillRect(x + 1, y + 1, width - 2, height - 2, color);

    mPainter->setPen(color.light());
    mPainter->drawLine(x, y + height - 1, x, y);
    mPainter->drawLine(x, y, x + width - 1, y);

    mPainter->setPen(color.dark());
    mPainter->drawLine(x + 1, y + height - 1, x + width - 1, y + height - 1);
    mPainter->drawLine(x + width - 1, y + height - 1, x + width - 1, y + 1);
}




void TetrisWidget::drawLine(int x1, int y1, int x2, int y2, int inPenWidth, const RGBColor & inColor)
{
    if (!mPainter.get())
    {
        throw std::logic_error("Painter is not set.");
    }

    mPainter->setPen(QColor(inColor.red(), inColor.green(), inColor.blue()));
    mPainter->drawLine(x1, y1, x2, y2);
}


Tetris::Rect TetrisWidget::getGameRect() const
{
    return Tetris::Rect(0,
                        0,
                        mColCount * squareWidth(),
                        mRowCount * squareHeight());
}


Tetris::Rect TetrisWidget::getFutureBlocksRect(unsigned int inFutureBlockCount) const
{
    int blockHeight = 3 * squareHeight();
    return Tetris::Rect(mColCount * squareHeight() + cMargin,
                        0,
                        4 * squareWidth(),
                        inFutureBlockCount * blockHeight);
}


void TetrisWidget::paintEvent(QPaintEvent * )
{
    if (!getGame())
    {
        return;
    }

    mPainter.reset(new QPainter(this));
    coordinateRepaint(*getGame());
    mPainter.reset();
}


QSize TetrisWidget::minimumSizeHint() const
{
    return mMinSize;
}
