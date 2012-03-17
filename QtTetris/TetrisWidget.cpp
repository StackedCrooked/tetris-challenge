#include "TetrisWidget.h"
#include "Tetris/SimpleGame.h"
#include "Futile/AutoPtrSupport.h"
#include "Futile/Logging.h"
#include "Futile/Threading.h"
#include <QtGui/QApplication>
#include <QtGui/QColor>
#include <QtGui/QKeyEvent>
#include <boost/noncopyable.hpp>
#include <stdexcept>
#include <iostream>


using namespace Futile;
using namespace Tetris;


namespace { // anonymous namespace


struct RestorePainter : boost::noncopyable
{
    RestorePainter(QPainter & inPainter) :
        mPainter(inPainter),
        mFont(inPainter.font()),
        mPen(inPainter.pen())
    {
    }

    ~RestorePainter()
    {
        mPainter.setFont(mFont);
        mPainter.setPen(mPen);
    }

    QPainter & mPainter;
    QFont mFont;
    QPen mPen;
};


struct RefreshTimer
{
    enum
    {
        cRefreshRate = 50, // fps
        cTimerInterval = 1000 / cRefreshRate
    };

    boost::signals2::signal<void()> OnTimer;

    RefreshTimer() :
        mTimer(cTimerInterval)
    {
        mTimer.start([&](){ OnTimer(); });
    }

private:
    Futile::Timer mTimer;
};


static boost::scoped_ptr<RefreshTimer> gSharedTimer;


struct NullDeleter
{
    void operator()(void const *) const
    {
    }
};


static unsigned sTetrisWidgetInstanceCount = 0;


} // anomymous namespace


struct TetrisWidget::Impl : boost::noncopyable
{

    Impl(TetrisWidget & inTetrisWidget) :
        mTetrisWidget(inTetrisWidget),
        mThis(this, NullDeleter()),
        mWeakPtr(mThis),
        mConnection()
    {
        if (++sTetrisWidgetInstanceCount == 1)
        {
            Assert(!gSharedTimer);
            gSharedTimer.reset(new RefreshTimer);
        }

    }

    ~Impl()
    {
        if (--sTetrisWidgetInstanceCount == 0)
        {
            Assert(gSharedTimer);
            gSharedTimer.reset();
        }
    }

    typedef boost::weak_ptr<Impl> WeakPtr;

    WeakPtr getWeakPtr()
    {
        return WeakPtr(mThis);
    }

    // Schedules a call to 'sendRefresh' on the main thread.
    // We must do it this way because we are using threaded callbacks.
    void postRefresh()
    {
        InvokeLater(boost::bind(&Impl::sendRefresh, mWeakPtr));
    }

    // Performs an immediate referesh.
    static void sendRefresh(WeakPtr inWeakPtr)
    {
        typedef boost::shared_ptr<Impl> SharedPtr;
        if (SharedPtr sharedPtr = inWeakPtr.lock())
        {
            Impl & impl = *sharedPtr;
            impl.mTetrisWidget.setUpdatesEnabled(false);
            impl.mTetrisWidget.repaint();
            impl.mTetrisWidget.setUpdatesEnabled(true);
        }
    }

    TetrisWidget & mTetrisWidget;
    boost::shared_ptr<Impl> mThis;
    WeakPtr mWeakPtr;

    QSize mMinSize;
    std::auto_ptr<QPainter> mPainter;
    boost::scoped_ptr<QImage> mImage;
    std::string mImageFileName;

    typedef boost::signals2::scoped_connection ScopedConnection;
    ScopedConnection mConnection;
};


TetrisWidget::TetrisWidget(QWidget * inParent, int inSquareWidth, int inSquareHeight) :
    QWidget(inParent),
    AbstractWidget(inSquareWidth, inSquareHeight),
    mImpl(new Impl(*this))
{
    Assert(gSharedTimer);
    if (gSharedTimer)
    {
        mImpl->mConnection = gSharedTimer->OnTimer.connect(boost::bind(&Impl::postRefresh, mImpl.get()));
    }
    setFocusPolicy(Qt::StrongFocus);
}


TetrisWidget::~TetrisWidget()
{
    destroy();
}


void TetrisWidget::destroy()
{
    try
    {
        mImpl->mConnection.release();
        mImpl.reset();
    }
    catch (const std::exception & exc)
    {
        std::cerr << "Caught exception in ~TetrisWidget: " << exc.what() << std::endl;
    }
}


void TetrisWidget::refresh()
{
    mImpl->postRefresh();
}


void TetrisWidget::keyPressEvent(QKeyEvent * inEvent)
{
    if (!game() ||
         game()->isPaused() ||
         game()->isGameOver() ||
         game()->playerType() == PlayerType_Computer)
    {
        QWidget::keyPressEvent(inEvent);
        return;
    }

    switch (inEvent->key())
    {
        case Qt::Key_Left:
        {
            game()->move(MoveDirection_Left);
            break;
        }
        case Qt::Key_Right:
        {
            game()->move(MoveDirection_Right);
            break;
        }
        case Qt::Key_Down:
        {
            game()->move(MoveDirection_Down);
            break;
        }
        case Qt::Key_Up:
        {
            game()->rotate();
            break;
        }
        case Qt::Key_Space:
        {
            game()->drop();
            break;
        }
        default:
        {
            QWidget::keyPressEvent(inEvent);
            break;
        }
    }
}


void TetrisWidget::setMinSize(const Tetris::Size & inSize)
{
    mImpl->mMinSize = QSize(inSize.width(), inSize.height());
}


Tetris::Size TetrisWidget::getMinSize() const
{
    return Tetris::Size(mImpl->mMinSize.width(), mImpl->mMinSize.height());
}


void TetrisWidget::fillRect(const Tetris::Rect & inRect, const Tetris::RGBColor & inColor)
{
    QColor color(inColor.red(), inColor.green(), inColor.blue());
    int x = inRect.x();
    int y = inRect.y();
    int width = inRect.width();
    int height = inRect.height();

    mImpl->mPainter->fillRect(x, y, width, height, color);
}


void TetrisWidget::drawRect(const Tetris::Rect & inRect, const Tetris::RGBColor & inColor)
{
    RestorePainter restorePainter(*mImpl->mPainter);

    QColor color(inColor.red(), inColor.green(), inColor.blue());
    int x = inRect.x();
    int y = inRect.y();
    int width = inRect.width();
    int height = inRect.height();

    mImpl->mPainter->setPen(color);
    mImpl->mPainter->drawRect(x, y, width, height);
}


void TetrisWidget::paintSquare(const Rect & inRect, const RGBColor & inColor)
{
    RestorePainter restorePainter(*mImpl->mPainter);

    QColor color(inColor.red(), inColor.green(), inColor.blue());
    int x = inRect.x();
    int y = inRect.y();
    int width = inRect.width();
    int height = inRect.height();

    mImpl->mPainter->fillRect(x + 1, y + 1, width - 2, height - 2, color);

    mImpl->mPainter->setPen(color.light());
    mImpl->mPainter->drawLine(x, y + height - 1, x, y);
    mImpl->mPainter->drawLine(x, y, x + width - 1, y);

    mImpl->mPainter->setPen(color.dark());
    mImpl->mPainter->drawLine(x + 1, y + height - 1, x + width - 1, y + height - 1);
    mImpl->mPainter->drawLine(x + width - 1, y + height - 1, x + width - 1, y + 1);
}


void TetrisWidget::paintStatItem(const Tetris::Rect & inRect, const std::string & inName, const std::string & inValue)
{
    RestorePainter restorePainter(*mImpl->mPainter);

    QPainter & painter(*mImpl->mPainter);
    QFont oldFont(painter.font());

    // Paint the stats title
    QFont titleFont(oldFont);
    titleFont.setBold(true);
    painter.setFont(titleFont);

    int nameX = inRect.left() + (inRect.width() - painter.fontMetrics().width(inName.c_str()))/2;
    int nameY = inRect.top() + margin();
    drawText(nameX, nameY, inName);

    // Paint the stats value
    QFont valueFont(oldFont);
    painter.setFont(valueFont);

    int valueRectY = nameY + painter.fontMetrics().height();
    int valueRectHeight = inRect.bottom() - valueRectY;
    int valueX = inRect.left() + (inRect.width() - painter.fontMetrics().width(inValue.c_str()))/2;
    int valueY = valueRectY + (valueRectHeight - painter.fontMetrics().height())/2;
    drawText(valueX, valueY, inValue);
}


void TetrisWidget::paintImage(const Tetris::Rect & inRect, const std::string & inFileName)
{
    if (!mImpl->mImage || mImpl->mImageFileName != inFileName)
    {
        mImpl->mImageFileName = inFileName;
        mImpl->mImage.reset(new QImage(std::string(":/resources/avatar/" + mImpl->mImageFileName).c_str()));
    }

    if (!mImpl->mImage->isNull())
    {
        QPainter & painter(*mImpl->mPainter);
        RestorePainter restorePainter(*mImpl->mPainter);

        QRectF rectF(inRect.left(), inRect.top(), inRect.width(), inRect.height());
        painter.fillRect(rectF, QColor(255, 255, 255));
        painter.drawImage(rectF, *mImpl->mImage);
    }
}


void TetrisWidget::drawLine(int x1, int y1, int x2, int y2, const RGBColor & inColor)
{
    RestorePainter restorePainter(*mImpl->mPainter);

    mImpl->mPainter->setPen(QColor(inColor.red(), inColor.green(), inColor.blue()));

    mImpl->mPainter->drawLine(x1, y1, x2, y2);
}


void TetrisWidget::drawText(int x, int y, const std::string & inText)
{
    RestorePainter restorePainter(*mImpl->mPainter);
    QPainter & painter(*mImpl->mPainter);

    painter.setPen(QColor(0, 0, 0));
    painter.drawText(QRect(x, y, game()->gameGrid().columnCount() * squareWidth(), squareHeight()), inText.c_str());
}


void TetrisWidget::drawTextCentered(const Rect & inRect, const std::string & inText, int inFontSize, const RGBColor & inColor)
{
    RestorePainter restorePainter(*mImpl->mPainter);
    QPainter & painter(*mImpl->mPainter);

    // Set the new pen
    painter.setPen(QColor(inColor.red(), inColor.green(), inColor.blue()));

    // Paint the stats title
    QFont textFont(painter.font());
    textFont.setPointSize(inFontSize);
    textFont.setBold(true);
    painter.setFont(textFont);

    int textWidth = painter.fontMetrics().width(inText.c_str());
    int textHeight = painter.fontMetrics().height();

    int x = inRect.left() + (inRect.width()  - textWidth)/2;
    int y = inRect.top()  + (inRect.height() - textHeight)/2;
    QRect theTextRect(x, y, game()->gameGrid().columnCount() * squareWidth(),squareHeight());
    painter.drawText(theTextRect, inText.c_str());
}


void TetrisWidget::drawTextRightAligned(const Rect & inRect, const std::string & inText, int inFontSize, const RGBColor & inColor)
{
    RestorePainter restorePainter(*mImpl->mPainter);
    QPainter & painter(*mImpl->mPainter);

    painter.setPen(QColor(inColor.red(), inColor.green(), inColor.blue()));

    // Paint the stats title
    QFont textFont(painter.font());
    textFont.setPointSize(inFontSize);
    textFont.setBold(true);
    painter.setFont(textFont);

    int textWidth = painter.fontMetrics().width(inText.c_str());
    int textHeight = painter.fontMetrics().height();

    int x = inRect.right() - textWidth - margin();
    int y = inRect.top()  + (inRect.height() - textHeight)/2;
    drawText(x, y, inText);
}


void TetrisWidget::paintEvent(QPaintEvent * )
{
    if (!game())
    {
        return;
    }
    mImpl->mPainter.reset(new QPainter(this));
    coordinateRepaint(*game());
    mImpl->mPainter.reset();
}


QSize TetrisWidget::minimumSizeHint() const
{
    return mImpl->mMinSize;
}
