#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QtGui>
#include "TetrisWidget.h"
#include "Tetris/SimpleGame.h"
#include "Tetris/MultiplayerGame.h"
#include "Tetris/Player.h"
#include "Tetris/PlayerType.h"
#include <boost/scoped_ptr.hpp>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static MainWindow * GetInstance();

    MainWindow(QWidget *parent = 0);

    ~MainWindow();

    virtual bool event(QEvent * inEvent);

    void logMessage(const std::string & inMessage);

private slots:
    void onNewSinglePlayerGame();

    void onNewHumanVsComputerGame();

    void onNewComputerVsComputerGame();

    void on2v2Game();

    void onPaused();

    void onPenalty();

    void onTimerEvent();

private:
    void onNewGame(const Tetris::PlayerTypes & inPlayerTypes);

    static MainWindow * sInstance;

private:
    typedef std::vector<TetrisWidget *> TetrisWidgets;
    QHBoxLayout * mTetrisWidgetHolder;
    TetrisWidgets mTetrisWidgets;
    int mSpacing;
    QTextEdit * mLogField;
    bool mShowLog;
    bool mGameOver;
};


#endif // MAINWINDOW_H
