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
    mTetrisWidget(),
    mFPSLabel(0),
    mRestartButton(0)
{
    mTetrisWidget = new TetrisWidget(this, cSquareWidth, cSquareHeight);
    mTetrisWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mFPSLabel = new QLabel(this);
    mRestartButton = new QPushButton("Restart", this);
    connect(mRestartButton, SIGNAL(clicked()), this, SLOT(onRestart()));


    QVBoxLayout * vbox = new QVBoxLayout(this);
    vbox->addWidget(mTetrisWidget);
    vbox->addWidget(mFPSLabel);
    vbox->addWidget(mRestartButton, 1);

    restart();

    // Refresh the FPS
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    timer->start(30);
}


MainWindow::~MainWindow()
{
}


void MainWindow::onRestart()
{
    restart();
}


void MainWindow::restart()
{
    // Make sure the previous game has been
    // deleted before creating a new one.
    mTetrisWidget->setGame(0);
    mSimpleGame.reset();
    mSimpleGame.reset(new SimpleGame(cRowCount, cColumnCount));
    mTetrisWidget->setGame(mSimpleGame.get());
}


void MainWindow::onTimeout()
{
    mFPSLabel->setText(QString("FPS: ") + QString::number(mTetrisWidget->getFPS()));
}
