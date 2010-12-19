#include "TetrisWidget.h"
#include "Tetris/SimpleGame.h"
#include <QColor>
#include <QPainter>
#include <QTimer>
#include <stdexcept>
#include <iostream>


using namespace Tetris;


extern const int cDefaultSquareSize = 20;
extern const int cDefaultRowCount = 20;
extern const int cDefaultColCount = 10;


TetrisWidget::TetrisWidget(QWidget * inParent) :
    QWidget(inParent),
    mSimpleGame(0),
    mSquareSize(cDefaultSquareSize),
    mSize(cDefaultColCount * cDefaultSquareSize,
          cDefaultRowCount * cDefaultSquareSize)
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
    mSize = QSize(cDefaultColCount * cDefaultSquareSize,
                  cDefaultRowCount * cDefaultSquareSize);
}


const QColor & TetrisWidget::getBlockColor(int inBlockType) const
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


void TetrisWidget::paintEvent(QPaintEvent * )
{
    QPainter painter(this);
    painter.fillRect(contentsRect(), getBlockColor(BlockType_Nil));

    if (!mSimpleGame)
    {
        return;
    }

    int w = mSquareSize;
    int h = mSquareSize;

    QRect unitRect(0, 0, w, h);
    Grid gameGrid = mSimpleGame->gameGrid();
    for (unsigned int c = 0; c < gameGrid.columnCount(); ++c)
    {
        for (unsigned int r = 0; r < gameGrid.rowCount(); ++r)
        {
            unitRect = QRect(c * w, r * h, w, h);
            BlockType blockType = gameGrid.get(r, c);
            if (blockType != BlockType_Nil)
            {
                const QColor & color(getBlockColor(blockType));
                painter.fillRect(unitRect, color);
            }
        }
    }

    Block activeBlock(mSimpleGame->activeBlock());
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
                const QColor & color(getBlockColor(blockType));
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
