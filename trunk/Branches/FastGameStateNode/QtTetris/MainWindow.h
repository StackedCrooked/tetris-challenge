#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QtGui/QtGui>
#include "Model.h"
#include "TetrisWidget.h"
#include "Tetris/Game.h"
#include "Tetris/MultiplayerGame.h"
#include "Tetris/Player.h"
#include "Tetris/PlayerType.h"
#include "Futile/Singleton.h"
#include "Poco/Stopwatch.h"
#include <boost/scoped_ptr.hpp>


class MainWindow : public QMainWindow,
                   public Futile::Singleton<MainWindow>
{
    Q_OBJECT

public:

    virtual bool event(QEvent * inEvent);

    void logMessage(const std::string & inMessage);

protected:
    void closeEvent(QCloseEvent*);

private slots:
    void onNew();

    void onNewSingleHumanPlayerGame();

    void onNewSingleComputerPlayerGame();

    void onNewHumanVsComputerGame();

    void onNewComputerVsComputerGame();

    void on2v2Game();

    void onPaused();

    void onPenalty();

    void onTimerEvent();

private:
    friend class Futile::Singleton<MainWindow>;

    MainWindow(QWidget *parent = 0);

    ~MainWindow();

    void onNewGame(const Tetris::PlayerTypes & inPlayerTypes);

    void timerEvent();

private:
    typedef std::vector<TetrisWidget *> TetrisWidgets;
    QHBoxLayout * mTetrisWidgetHolder;
    TetrisWidgets mTetrisWidgets;
    int mSpacing;
    QTextEdit * mLogField;
    bool mShowLog;
    bool mGameOver;
    Poco::Stopwatch mStopwatch;
};


#endif // MAINWINDOW_H