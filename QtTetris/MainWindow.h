#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QtGui>
#include <QMainWindow>
#include <QHBoxLayout>
#include <QTextEdit>
#include "Model.h"
#include "TetrisWidget.h"
#include "Tetris/Game.h"
#include "Tetris/MultiplayerGame.h"
#include "Tetris/Player.h"
#include "Tetris/PlayerType.h"
#include "Futile/Singleton.h"
#include "Poco/Stopwatch.h"
#include <boost/scoped_ptr.hpp>


class MainWindow final :
    public QMainWindow,
    public Futile::Singleton<MainWindow>
{
    Q_OBJECT

public:
    virtual bool event(QEvent* inEvent) override;

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

private:
    friend class Futile::Singleton<MainWindow>;

    MainWindow(QWidget *parent = 0);

    ~MainWindow();

    void onNewGame(const Tetris::PlayerTypes & inPlayerTypes);

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
