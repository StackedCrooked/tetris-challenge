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
    QWidget(parent),
    mSimpleGame(new Tetris::SimpleGame(cRowCount, cColumnCount))
{
    mTetrisWidget = new TetrisWidget(this, cUnitWidth, cUnitHeight);
    mTetrisWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mFPSLabel = new QLabel(this);

    QVBoxLayout * vbox = new QVBoxLayout(this);
    vbox->addWidget(mTetrisWidget);
    vbox->addWidget(mFPSLabel);

    mTetrisWidget->setSimpleGame(mSimpleGame);
    mSimpleGame->enableGravity(true);
    mSimpleGame->enableComputerPlayer(true);
    mSimpleGame->setComputerMoveSpeed(40);
    mSimpleGame->setLevel(6);


    // Refresh the FPS
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timer->start(30);
}


MainWindow::~MainWindow()
{
    delete mSimpleGame;
}


void MainWindow::onTimeout()
{
    mFPSLabel->setText(QString("FPS: ") + QString::number(mTetrisWidget->getFPS()));
}
