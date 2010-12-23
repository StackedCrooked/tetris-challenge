#include "MainWindow.h"
#include "Tetris/SimpleGame.h"
#include <QLayout>
#include <QLabel>
#include <iostream>


using namespace Tetris;


//
// Configuration
//
const int cRowCount(20);
const int cColumnCount(10);
const int cUnitWidth(20);
const int cUnitHeight(20);


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mSimpleGame(new Tetris::SimpleGame(cRowCount, cColumnCount))
{
    mTetrisWidget = new TetrisWidget(this, cUnitWidth, cUnitHeight);

    mTetrisWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setCentralWidget(mTetrisWidget);

    mTetrisWidget->setSimpleGame(mSimpleGame);
    mSimpleGame->enableGravity(true);
    mSimpleGame->enableComputerPlayer(true);
    mSimpleGame->setComputerMoveSpeed(40);
    mSimpleGame->setLevel(6);
}


MainWindow::~MainWindow()
{
    delete mSimpleGame;
}
