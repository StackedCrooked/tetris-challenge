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

private:
    std::auto_ptr<Tetris::SimpleGame> mSimpleGame;
    TetrisWidget * mTetrisWidget;
    QLabel * mFPSLabel;
};


#endif // MAINWINDOW_H