#ifndef TETRISWIDGET_H
#define TETRISWIDGET_H


#include "Tetris/AbstractWidget.h"
#include <QPainter>
#include <QWidget>


namespace Tetris {
class SimpleGame;
class Rect;
class RGBColor;
}


class TetrisWidget : public QWidget,
                     public Tetris::AbstractWidget
{
    Q_OBJECT
public:
    explicit TetrisWidget(QWidget * inParent, int inSquareWidth, int inSquareHeight);

    ~TetrisWidget();

    void setSimpleGame(Tetris::SimpleGame * inSimpleGame);

    virtual void paintEvent(QPaintEvent * event);

    virtual QSize minimumSizeHint() const;

    virtual void paintSquare(const Tetris::Rect & inRect, const Tetris::RGBColor & inColor);

    virtual void drawLine(int x1, int y1, int x2, int y2, int inPenWidth, const Tetris::RGBColor & inColor);

    virtual Tetris::Rect getGameRect() const;

    virtual Tetris::Rect getFutureBlocksRect(unsigned int inFutureBlockCount) const;

signals:

public slots:

private:
    Tetris::SimpleGame * mSimpleGame;
    int mRowCount;
    int mColCount;
    QSize mMinSize;
    std::auto_ptr<QPainter> mPainter;
};


#endif // TETRISWIDGET_H
