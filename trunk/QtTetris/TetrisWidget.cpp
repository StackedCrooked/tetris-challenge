#include "TetrisWidget.h"
#include "Tetris/Game.h"
#include "Tetris/SimpleGame.h"
#include "Futile/Threading.h"
#include "Futile/AutoPtrSupport.h"
#include <QtGui/QApplication>
#include <QKeyEvent>
#include <QColor>
#include <QMutexLocker>
#include <QPainter>
#include <QTimer>
#include <stdexcept>
#include <iostream>


using namespace Tetris;


std::set<TetrisWidget*> TetrisWidget::sInstances;


TetrisWidget::TetrisWidget(QWidget * inParent, int inSquareWidth, int inSquareHeight) :
    QWidget(inParent),
    AbstractWidget(inSquareWidth, inSquareHeight),
    mMinSize(),
    mPainter()
{
    setUpdatesEnabled(true);
    setFocusPolicy(Qt::StrongFocus);
    sInstances.insert(this);
}


TetrisWidget::~TetrisWidget()
{
    sInstances.erase(this);
}


void TetrisWidget::refresh()
{
    update();
}


void TetrisWidget::keyPressEvent(QKeyEvent * inEvent)
{
    if (!simpleGame() ||
         simpleGame()->isPaused() ||
         simpleGame()->isGameOver() ||
         simpleGame()->playerType() == PlayerType_Computer)
    {
        QWidget::keyPressEvent(inEvent);
        return;
    }

    switch (inEvent->key())
    {
        case Qt::Key_Left:
        {
            simpleGame()->move(MoveDirection_Left);
            break;
        }
        case Qt::Key_Right:
        {
            simpleGame()->move(MoveDirection_Right);
            break;
        }
        case Qt::Key_Down:
        {
            simpleGame()->move(MoveDirection_Down);
            break;
        }
        case Qt::Key_Up:
        {
            simpleGame()->rotate();
            break;
        }
        case Qt::Key_Space:
        {
            simpleGame()->drop();
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
    mMinSize = QSize(inSize.width(), inSize.height());
}


Tetris::Size TetrisWidget::getMinSize() const
{
    return Tetris::Size(mMinSize.width(), mMinSize.height());
}


void TetrisWidget::fillRect(const Tetris::Rect & inRect, const Tetris::RGBColor & inColor)
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

    mPainter->fillRect(x, y, width, height, color);
}


void TetrisWidget::drawRect(const Tetris::Rect & inRect, const Tetris::RGBColor & inColor)
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

    mPainter->setPen(color);
    mPainter->drawRect(x, y, width, height);
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


void TetrisWidget::paintStatItem(const Tetris::Rect & inRect, const std::string & inName, const std::string & inValue)
{
    if (!mPainter.get())
    {
        throw std::logic_error("Painter is not set.");
    }

    QPainter & painter(*mPainter);
    QFont oldFont(painter.font());

    // Paint the stats title
    QFont titleFont(oldFont);
    titleFont.setBold(true);
    titleFont.setWeight(QFont::Black);
    painter.setFont(titleFont);

    int nameX = inRect.left() + (inRect.width() - painter.fontMetrics().width(inName.c_str()))/2;
    int nameY = inRect.top() + margin();
    drawText(nameX, nameY, inName);

    // Paint the stats value
    QFont valueFont(oldFont);
    valueFont.setWeight(QFont::Bold);
    painter.setFont(valueFont);

    int valueRectY = nameY + painter.fontMetrics().height();
    int valueRectHeight = inRect.bottom() - valueRectY;
    int valueX = inRect.left() + (inRect.width() - painter.fontMetrics().width(inValue.c_str()))/2;
    int valueY = valueRectY + (valueRectHeight - painter.fontMetrics().height())/2;
    drawText(valueX, valueY, inValue);
}


void TetrisWidget::paintImage(const Tetris::Rect & inRect, const std::string & inFileName)
{
    if (!mImage || mImageFileName != inFileName)
    {
        mImageFileName = inFileName;
        mImage.reset(new QImage(inFileName.c_str()));
    }

    if (!mImage->isNull())
    {
        QPainter & painter(*mPainter);
        QRectF rectF(inRect.left(), inRect.top(), inRect.width(), inRect.height());
        painter.fillRect(rectF, QColor(255, 255, 255));
        painter.drawImage(rectF, *mImage);
    }
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


void TetrisWidget::drawText(int x, int y, const std::string & inText)
{
    if (!mPainter.get())
    {
        throw std::logic_error("Painter is not set.");
    }

    QPainter & painter(*mPainter);
    painter.setPen(QColor(0, 0, 0));
    painter.drawText(QRect(x, y, simpleGame()->gameGrid().columnCount() * squareWidth(), squareHeight()), inText.c_str());
}


void TetrisWidget::drawTextCentered(const Rect & inRect, const std::string & inText, int inFontSize, const RGBColor & inColor)
{
    if (!mPainter.get())
    {
        throw std::logic_error("Painter is not set.");
    }

    QPainter & painter(*mPainter);
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
    drawText(x, y, inText);
}


void TetrisWidget::drawTextRightAligned(const Rect & inRect, const std::string & inText, int inFontSize, const RGBColor & inColor)
{
    if (!mPainter.get())
    {
        throw std::logic_error("Painter is not set.");
    }

    QPainter & painter(*mPainter);
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
    if (!simpleGame())
    {
        return;
    }

    mPainter.reset(new QPainter(this));
    coordinateRepaint(*simpleGame());
    mPainter.reset();
}


QSize TetrisWidget::minimumSizeHint() const
{
    return mMinSize;
}
