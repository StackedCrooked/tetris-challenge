#include "MainWindow.h"
#include <QLayout>
#include <QLabel>
#include <iostream>


using namespace Tetris;


static Protected<Game> Tetris_CreateGame()
{
    return Protected<Game>(Create<Game>(TetrisWidget_NumRows(),
                                        TetrisWidget_NumColumns()));
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mGame(Tetris_CreateGame()),
    mGravityCallback(),
    mGravity(mGame),
    mTetrisWidget(0)
{
    QWidget * centralWidget = new QWidget(this);

    mTetrisWidget = new TetrisWidget(centralWidget, mGame);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(mTetrisWidget, 1, Qt::AlignCenter);

    centralWidget->setLayout(hbox);
    setCentralWidget(centralWidget);

    mGravityCallback.reset(new GravityCallback<MainWindow>(this));
    mGravity.setGravityCallback(mGravityCallback.get());
}


MainWindow::~MainWindow()
{
    mGravity.setGravityCallback(NULL);
}


void MainWindow::onGravityCallback(Gravity * )
{
    mTetrisWidget->update();
}

