#include "MainWindow.h"
#include "Tetris/SimpleGame.h"
#include <QLayout>
#include <QLabel>
#include <iostream>


using namespace Tetris;


extern const int cDefaultRowCount;
extern const int cDefaultColCount;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mSimpleGame(new Tetris::SimpleGame(cDefaultRowCount, cDefaultColCount))
{
    QWidget * centralWidget = new QWidget(this);

    mTetrisWidget = new TetrisWidget(centralWidget);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(mTetrisWidget, 1, Qt::AlignCenter);

    centralWidget->setLayout(hbox);
    setCentralWidget(centralWidget);

    mTetrisWidget->setSimpleGame(mSimpleGame);
    mSimpleGame->enableGravity(true);
    mSimpleGame->enableComputerPlayer(true);
    mSimpleGame->setComputerMoveSpeed(100);
    mSimpleGame->setLevel(10);
}


MainWindow::~MainWindow()
{
    delete mSimpleGame;
}
