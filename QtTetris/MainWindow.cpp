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
    mGravity(),
    mBlockMover(),
    mComputerPlayer(),
    mTetrisWidget(0)
{
    QWidget * centralWidget = new QWidget(this);

    mTetrisWidget = new TetrisWidget(centralWidget, mGame);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(mTetrisWidget, 1, Qt::AlignCenter);

    centralWidget->setLayout(hbox);
    setCentralWidget(centralWidget);


    mGravity.reset(new Gravity(mGame));
    mBlockMover.reset(new BlockMover(mGame));
    std::auto_ptr<Evaluator> evaluator(new Balanced);
    mComputerPlayer.reset(new ComputerPlayer(mGame, evaluator, 6, 6, 4));
}


MainWindow::~MainWindow()
{
    mGravity->setCallback(NULL);
}
