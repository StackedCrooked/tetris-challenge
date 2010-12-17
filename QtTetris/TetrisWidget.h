#ifndef TETRISWIDGET_H
#define TETRISWIDGET_H


#include <QWidget>
#include "Tetris/Game.h"
#include "Tetris/Threading.h"


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

    virtual void paintEvent(QPaintEvent * event);

    virtual QSize sizeHint() const;

    virtual QSize minimumSizeHint() const;

signals:

public slots:

private:
    Protected<Game> mGame;
    QSize mSize;
};


} // namespace Tetris


#endif // TETRISWIDGET_H
