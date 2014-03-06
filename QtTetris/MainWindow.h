#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#ifndef QT_NO_KEYWORDS
#define QT_NO_KEYWORDS
#endif

#ifndef QT_NO_DEBUG
#define QT_NO_DEBUG
#endif


// INFO: Fixes Clang build error. Maybe this can be removed in the future.
#ifndef QT_NO_STL
#define QT_NO_STL
#endif // QT_NO_STL


#include <QtGui>
#include "Model.h"
#include "TetrisWidget.h"
#include "Tetris/SimpleGame.h"
#include "Tetris/MultiplayerGame.h"
#include "Tetris/Player.h"
#include "Tetris/PlayerType.h"
#include "Poco/Stopwatch.h"
#include <boost/scoped_ptr.hpp>


namespace QtTetris {


using namespace Tetris;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget * inParent, Model & inModel);

    ~MainWindow();

    virtual bool event(QEvent * inEvent);

    void logMessage(const std::string & inMessage);

protected:
    void closeEvent(QCloseEvent*);

private Q_SLOTS:
    void onNew();

    void onNewSingleHumanPlayerGame();

    void onNewSingleComputerPlayerGame();

    void onNewHumanVsComputerGame();

    void onNewComputerVsComputerGame();

    void on2v2Game();

    void onPaused();

    void onPenalty();

    void onExit();

    void onTimerEvent();

private:
    void onNewGame(const Tetris::PlayerTypes & inPlayerTypes);

    void timerEventImpl();

private:
    Model & mModel;
    typedef std::vector<TetrisWidget *> TetrisWidgets;
    QHBoxLayout * mTetrisWidgetHolder;
    TetrisWidgets mTetrisWidgets;
    int mSpacing;
    QTextEdit * mLogField;
    bool mShowLog;
    bool mGameOver;
};


} // namespace QtTetris


#endif // MAINWINDOW_H
