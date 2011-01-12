#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QtGui>
#include "TetrisWidget.h"
#include "Tetris/SimpleGame.h"
#include <memory>


class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onTimeout();

    void onRestart();

private:
    void restart();

    std::auto_ptr<Tetris::SimpleGame> mSimpleGame1;
    std::auto_ptr<Tetris::SimpleGame> mSimpleGame2;
    TetrisWidget * mTetrisWidget1;
    TetrisWidget * mTetrisWidget2;
    QLabel * mFPSLabel;
    QPushButton * mRestartButton;
};


#endif // MAINWINDOW_H
