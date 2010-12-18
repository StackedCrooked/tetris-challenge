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

    Tetris::Protected<Tetris::Game> mGame;

    std::auto_ptr<Tetris::Gravity> mGravity;
    std::auto_ptr<Tetris::BlockMover> mBlockMover;
    std::auto_ptr<Tetris::ComputerPlayer> mComputerPlayer;
    Tetris::TetrisWidget * mTetrisWidget;
};


#endif // MAINWINDOW_H
