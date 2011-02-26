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


class TetrisWidget : public QWidget,
                     public Tetris::AbstractWidget
{
    Q_OBJECT
public:
    explicit TetrisWidget(QWidget * inParent, int inSquareWidth, int inSquareHeight);

    virtual ~TetrisWidget();

    virtual void keyPressEvent(QKeyEvent * inKeyEvent);

    virtual void refresh();

protected:
    virtual void setMinSize(const Tetris::Size & inSize);
    virtual Tetris::Size getMinSize() const;
    virtual void fillRect(const Tetris::Rect & inRect, const Tetris::RGBColor & inColor);
    virtual void drawRect(const Tetris::Rect & inRect, const Tetris::RGBColor & inColor);
    virtual void drawLine(int x1, int y1, int x2, int y2, int inPenWidth, const Tetris::RGBColor & inColor);
    virtual void drawText(int x, int y, const std::string & inText);
    virtual void drawTextCentered(const Tetris::Rect & inRect, const std::string & inText, int inFontSize, const Tetris::RGBColor & inColor);
    virtual void drawTextRightAligned(const Tetris::Rect & inRect, const std::string & inText, int inFontSize, const Tetris::RGBColor & inColor);
    virtual void paintSquare(const Tetris::Rect & inRect, const Tetris::RGBColor & inColor);
    virtual void paintStatItem(const Tetris::Rect & inRect, const std::string & inName, const std::string & inValue);
    virtual void paintImage(const Tetris::Rect & inRect, const std::string & inFileName);

private:
    virtual void paintEvent(QPaintEvent * event);
    virtual QSize minimumSizeHint() const;

    QSize mMinSize;
    std::auto_ptr<QPainter> mPainter;

    friend void Tetris::InvokeLater(const Tetris::Action &);

    typedef std::set<TetrisWidget*> Instances;
    static Instances sInstances;

    boost::scoped_ptr<QImage> mImage;
    std::string mImageFileName;
};


#endif // TETRISWIDGET_H
