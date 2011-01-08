#include "MainWindow.h"
#include <QLayout>
#include <QLabel>
#include <iostream>


using namespace Tetris;


//
// Configuration
//
const int cRowCount(20);
const int cColumnCount(10);
const int cSquareWidth(20);
const int cSquareHeight(20);


MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    mSimpleGame(),
    mTetrisWidget()
{
    mTetrisWidget = new TetrisWidget(this, cSquareWidth, cSquareHeight);
    mSimpleGame.reset(new SimpleGame(mTetrisWidget, cRowCount, cColumnCount));
    mTetrisWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mFPSLabel = new QLabel(this);

    QVBoxLayout * vbox = new QVBoxLayout(this);
    vbox->addWidget(mTetrisWidget);
    vbox->addWidget(mFPSLabel);

    mTetrisWidget->setGame(mSimpleGame.get());


    // Refresh the FPS
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timer->start(30);
}


MainWindow::~MainWindow()
{
}


void MainWindow::onTimeout()
{
    mFPSLabel->setText(QString("FPS: ") + QString::number(mTetrisWidget->getFPS()));
}
