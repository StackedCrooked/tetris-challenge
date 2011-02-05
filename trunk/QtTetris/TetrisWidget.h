#ifndef TETRISWIDGET_H
#define TETRISWIDGET_H


#include "Tetris/AbstractWidget.h"
#include "Tetris/Threading.h"
#include <QPainter>
#include <QMutex>
#include <QWidget>
#include <set>


namespace Tetris {
class Rect;
class RGBColor;
}


class ActionEvent;


class TetrisWidget : public QWidget,
                     public Tetris::AbstractWidget
{
    Q_OBJECT
public:
    explicit TetrisWidget(QWidget * inParent, int inSquareWidth, int inSquareHeight);

    ~TetrisWidget();

    virtual void keyPressEvent(QKeyEvent * inKeyEvent);

    virtual void refresh();

protected:
    virtual void setMinSize(int inWidth, int inHeight);
    virtual void paintSquare(const Tetris::Rect & inRect, const Tetris::RGBColor & inColor);
    virtual void drawLine(int x1, int y1, int x2, int y2, int inPenWidth, const Tetris::RGBColor & inColor);
    virtual void drawText(int x, int y, const std::string & inText);
    virtual Tetris::Rect getGameRect() const;
    virtual Tetris::Rect getStatsRect() const;
    virtual Tetris::Rect getFutureBlocksRect(unsigned int inFutureBlockCount) const;

private:
    virtual void paintEvent(QPaintEvent * event);
    virtual QSize minimumSizeHint() const;

    void clearGameState();
    void toggleActiveBlock();

    int mRowCount;
    int mColCount;
    QSize mMinSize;
    std::auto_ptr<QPainter> mPainter;

    friend class ActionEvent;
    friend void Tetris::InvokeLater(const Tetris::Action &);

    typedef std::set<TetrisWidget*> Instances;
    static Instances sInstances;
};


#endif // TETRISWIDGET_H
