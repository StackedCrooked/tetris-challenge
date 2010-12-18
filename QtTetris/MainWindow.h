#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QtGui/QMainWindow>
#include "TetrisWidget.h"
#include <memory>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void onGravityCallback(Tetris::Gravity * inGravity);

    Tetris::Protected<Tetris::Game> mGame;
    std::auto_ptr<Tetris::AbstractGravityCallback> mGravityCallback;
    Tetris::Gravity mGravity;
    Tetris::TetrisWidget * mTetrisWidget;
};


#endif // MAINWINDOW_H
