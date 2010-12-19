#ifndef TETRISWIDGET_H
#define TETRISWIDGET_H


#include <QWidget>
#include "Tetris/Tetris.h"


//
// Must be defined by the user
//
int Tetris_GetUnitWidth();
int Tetris_GetUnitHeight();
int TetrisWidget_NumColumns();
int TetrisWidget_NumRows();


namespace Tetris {


class Game;



class TetrisWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TetrisWidget(QWidget * inParent, const Protected<Game> & inGame);

    explicit TetrisWidget(QWidget * inParent);

    virtual void paintEvent(QPaintEvent * event);

    virtual QSize sizeHint() const;

    virtual QSize minimumSizeHint() const;

signals:

public slots:

private:
    const QColor & getColor(BlockType inBlockType) const;

    void init();

    Protected<Game> mGame;
    std::auto_ptr<Gravity> mGravity;
    std::auto_ptr<BlockMover> mBlockMover;
    std::auto_ptr<ComputerPlayer> mComputerPlayer;
    QSize mSize;
};


} // namespace Tetris


#endif // TETRISWIDGET_H
