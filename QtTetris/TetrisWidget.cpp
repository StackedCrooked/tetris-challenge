#include "TetrisWidget.h"
#include <QColor>
#include <QPainter>
#include <QTimer>
#include <stdexcept>
#include <iostream>


namespace Tetris {


static Protected<Game> Tetris_CreateGame()
{
    return Protected<Game>(Create<Game>(TetrisWidget_NumRows(),
                                        TetrisWidget_NumColumns()));
}


TetrisWidget::TetrisWidget(QWidget * inParent) :
    QWidget(inParent),
    mGame(Tetris_CreateGame()),
    mSize(Tetris_GetUnitWidth() * TetrisWidget_NumColumns(), Tetris_GetUnitHeight() * TetrisWidget_NumRows())
{
    init();

}


TetrisWidget::TetrisWidget(QWidget * inParent, const Protected<Game> & inGame) :
    QWidget(inParent),
    mGame(inGame),
    mSize(Tetris_GetUnitWidth() * TetrisWidget_NumColumns(), Tetris_GetUnitHeight() * TetrisWidget_NumRows())
{
    init();
}


void TetrisWidget::init()
{
    setUpdatesEnabled(true);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(30);

    mGravity.reset(new Gravity(mGame));
    mBlockMover.reset(new BlockMover(mGame));
    std::auto_ptr<Evaluator> evaluator(new Balanced);
    mComputerPlayer.reset(new ComputerPlayer(mGame, evaluator, 6, 6, 1));
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


void TetrisWidget::paintEvent(QPaintEvent * )
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
