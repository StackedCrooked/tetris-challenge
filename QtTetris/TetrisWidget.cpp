#include "TetrisWidget.h"
#include <QPainter>
#include <QColor>
#include <stdexcept>
#include <iostream>


namespace Tetris {


TetrisWidget::TetrisWidget(QWidget * inParent, const Protected<Game> & inGame) :
    QWidget(inParent),
    mGame(inGame),
    mSize(Tetris_GetUnitWidth() * TetrisWidget_NumColumns(), Tetris_GetUnitHeight() * TetrisWidget_NumRows())
{
    setUpdatesEnabled(true);
}


const QColor & TetrisWidget::getColor(Tetris::BlockType inBlockType) const
{
    if (inBlockType < BlockType_Nil || inBlockType >= BlockType_End)
    {
        throw std::logic_error("Invalid BlockType enum value.");
    }

    static const QColor fColors[] =
    {
        QColor(Qt::white),          // Background
        QColor(Qt::cyan),           // I-Shape
        QColor(Qt::blue),           // J-Shape
        QColor(255, 165, 0),        // L-Shape
        QColor(Qt::yellow),         // O-Shape
        QColor(Qt::green),          // S-Shape
        QColor(160, 32, 240),       // T-Shape
        QColor(Qt::red)             // Z-Shape
    };
    return fColors[static_cast<int>(inBlockType)];
}


void TetrisWidget::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    painter.fillRect(contentsRect(), getColor(BlockType_Nil));

    int w = Tetris_GetUnitWidth();
    int h = Tetris_GetUnitHeight();

    QRect unitRect(0, 0, w, h);
    const Grid & gameGrid = ScopedReader<Game>(mGame)->gameGrid();
    for (unsigned int c = 0; c < gameGrid.columnCount(); ++c)
    {
        for (unsigned int r = 0; r < gameGrid.rowCount(); ++r)
        {
            unitRect = QRect(c * w, r * h, w, h);
            BlockType blockType = gameGrid.get(r, c);
            if (blockType != BlockType_Nil)
            {
                const QColor & color(getColor(blockType));
                painter.fillRect(unitRect, color);
            }
        }
    }

    const Block & activeBlock(ScopedReader<Game>(mGame)->activeBlock());
    int offsetCol = activeBlock.column();
    int offsetRow = activeBlock.row();
    for (unsigned int c = 0; c < activeBlock.columnCount(); ++c)
    {
        for (unsigned int r = 0; r < activeBlock.rowCount(); ++r)
        {
            unitRect = QRect((offsetCol + c) * w, (offsetRow + r) * h, w, h);
            BlockType blockType = activeBlock.grid().get(r, c);
            if (blockType != BlockType_Nil)
            {
                const QColor & color(getColor(blockType));
                painter.fillRect(unitRect, color);
            }
        }
    }
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
