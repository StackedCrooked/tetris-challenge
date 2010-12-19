#ifndef TETRISWIDGET_H
#define TETRISWIDGET_H


#include <QWidget>


namespace Tetris {
class SimpleGame;
}


class TetrisWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TetrisWidget(QWidget * inParent);

    ~TetrisWidget();

    void setSimpleGame(Tetris::SimpleGame * inSimpleGame);

    virtual void paintEvent(QPaintEvent * event);

    virtual QSize sizeHint() const;

    virtual QSize minimumSizeHint() const;

protected:
    virtual const QColor & getBlockColor(int inBlockType) const;

signals:

public slots:

private:
    Tetris::SimpleGame * mSimpleGame;
    int mSquareSize;
    QSize mSize;
};


#endif // TETRISWIDGET_H
