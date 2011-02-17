#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QtGui>
#include "TetrisWidget.h"
#include "Tetris/SimpleGame.h"
#include "Tetris/MultiplayerGame.h"
#include "Tetris/Player.h"
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>


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
    void onRestart();

    void onPaused();

    void onPenalty();

private:
    static MainWindow * sInstance;

private:
    void restart();

    typedef std::vector<TetrisWidget *> TetrisWidgets;
    TetrisWidgets mTetrisWidgets;
    QPushButton * mRestartButton;
    QPushButton * mPauseButton;
    QPushButton * mPenaltyButton;
    QTextEdit * mLogField;
};


#endif // MAINWINDOW_H
