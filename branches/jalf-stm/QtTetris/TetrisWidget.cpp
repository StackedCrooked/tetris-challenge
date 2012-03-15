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


struct TimerHolder
{
    enum {
        cInterval = 16
    };

    boost::signals2::signal<void()> OnTimer;

    static TimerHolder & Get()
    {
        static TimerHolder fTimerHolder;
        return fTimerHolder;
    }

private:
    TimerHolder() :
        mTimer(std::make_shared<Timer>(cInterval))
    {
        mTimer->start([&](){ OnTimer(); });
    }

    std::shared_ptr<Futile::Timer> mTimer;
};


} // anomymous namespace


struct TetrisWidget::Impl
{
    template<typename Callback>
    Impl(const Callback & inCallback) :
        mConnection(inCallback)
    {
    }

    ~Impl()
    {
    }

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
    mImpl(new Impl(TimerHolder::Get().OnTimer.connect(boost::bind(&TetrisWidget::refresh, this))))
{
    setUpdatesEnabled(true);
    setFocusPolicy(Qt::StrongFocus);
}


TetrisWidget::~TetrisWidget()
{
    try
    {
        mImpl.reset();
    }
    catch (const std::exception & exc)
    {
        std::cerr << "Caught exception in ~TetrisWidget: " << exc.what() << std::endl;
    }
}


void TetrisWidget::refresh()
{
    InvokeLater(boost::bind(&TetrisWidget::update, this));
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
    if (!mImpl->mPainter.get())
    {
        throw std::logic_error("Painter is not set.");
    }

    QColor color(inColor.red(), inColor.green(), inColor.blue());
    int x = inRect.x();
    int y = inRect.y();
    int width = inRect.width();
    int height = inRect.height();

    mImpl->mPainter->fillRect(x, y, width, height, color);
}


void TetrisWidget::drawRect(const Tetris::Rect & inRect, const Tetris::RGBColor & inColor)
{
    if (!mImpl->mPainter.get())
    {
        throw std::logic_error("Painter is not set.");
    }
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
    if (!mImpl->mPainter.get())
    {
        throw std::logic_error("Painter is not set.");
    }

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
    if (!mImpl->mPainter.get())
    {
        throw std::logic_error("Painter is not set.");
    }
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
    if (!mImpl->mPainter.get())
    {
        throw std::logic_error("Painter is not set.");
    }
    RestorePainter restorePainter(*mImpl->mPainter);

    mImpl->mPainter->setPen(QColor(inColor.red(), inColor.green(), inColor.blue()));

    mImpl->mPainter->drawLine(x1, y1, x2, y2);
}


void TetrisWidget::drawText(int x, int y, const std::string & inText)
{
    if (!mImpl->mPainter.get())
    {
        throw std::logic_error("Painter is not set.");
    }

    RestorePainter restorePainter(*mImpl->mPainter);
    QPainter & painter(*mImpl->mPainter);

    painter.setPen(QColor(0, 0, 0));
    painter.drawText(QRect(x, y, game()->gameGrid().columnCount() * squareWidth(), squareHeight()), inText.c_str());
}


void TetrisWidget::drawTextCentered(const Rect & inRect, const std::string & inText, int inFontSize, const RGBColor & inColor)
{
    if (!mImpl->mPainter.get())
    {
        throw std::logic_error("Painter is not set.");
    }

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
    if (!mImpl->mPainter.get())
    {
        throw std::logic_error("Painter is not set.");
    }

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
