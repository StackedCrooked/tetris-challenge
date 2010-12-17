#include "TetrisWidget.h"
#include <stdexcept>
#include <QPainter>
#include <iostream>


#define TRACE std::cout << __PRETTY_FUNCTION__ << std::endl;


namespace Tetris {


TetrisWidget::TetrisWidget(QWidget * inParent, const Protected<Game> & inGame) :
    QWidget(inParent),
    mGame(inGame),
    mSize(Tetris_GetUnitWidth() * TetrisWidget_NumColumns(), Tetris_GetUnitHeight() * TetrisWidget_NumRows())
{
    TRACE
}

void TetrisWidget::paintEvent(QPaintEvent * event)
{
    QRect rect = contentsRect();
    QPixmap pixmap(rect.size());
    QPainter painter(this);
    painter.fillRect(pixmap.rect(), QColor(255, 255, 255));
}


QSize TetrisWidget::sizeHint() const
{
    return mSize;
}


QSize TetrisWidget::minimumSizeHint() const
{
    return mSize;
}


} // namespace Tetris
