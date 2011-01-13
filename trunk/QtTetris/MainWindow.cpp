#include "MainWindow.h"
#include "Tetris/Game.h"
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
    mSimpleGame1(),
    mSimpleGame2(),
    mTetrisWidget1(),
    mTetrisWidget2(),
    mFPSLabel(0),
    mSwitchButton(0),
    mRestartButton(0)
{
    mTetrisWidget1 = new TetrisWidget(this, cSquareWidth, cSquareHeight);
    mTetrisWidget1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mTetrisWidget2 = new TetrisWidget(this, cSquareWidth, cSquareHeight);
    mTetrisWidget2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mFPSLabel = new QLabel(this);

    mSwitchButton = new QPushButton("Switch", this);
    connect(mSwitchButton, SIGNAL(clicked()), this, SLOT(onRestart()));

    mRestartButton = new QPushButton("Restart", this);
    connect(mRestartButton, SIGNAL(clicked()), this, SLOT(onRestart()));

    QVBoxLayout * vbox = new QVBoxLayout(this);
    QHBoxLayout * hbox = new QHBoxLayout(this);
    hbox->addWidget(mTetrisWidget1);
    hbox->addWidget(mTetrisWidget2);

    vbox->addItem(hbox);

    vbox->addWidget(mSwitchButton);
    vbox->addWidget(mRestartButton);
    vbox->addWidget(mFPSLabel);
    vbox->addWidget(new QLabel("Press 'c' to clear the game."), 0);

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


void MainWindow::onSwapFields()
{
    try
    {
        swapFields();
    }
    catch(const std::exception & inException)
    {
        QString msg = "Exception thrown: ";
        msg += inException.what();
        QMessageBox::critical(this, "Tetris", msg);
    }
}


void MainWindow::swapFields()
{
    //mSimpleGame1->swap(mSimpleGame2);
}


void MainWindow::restart()
{
    // Make sure the previous game has been
    // deleted before creating a new one.
    mTetrisWidget1->setGame(0);
    mTetrisWidget2->setGame(0);
    mSimpleGame1.reset();
    mSimpleGame2.reset();
    mSimpleGame1.reset(new SimpleGame(cRowCount, cColumnCount));
    mSimpleGame2.reset(new SimpleGame(cRowCount, cColumnCount));
    mTetrisWidget1->setGame(mSimpleGame1.get());
    mTetrisWidget2->setGame(mSimpleGame2.get());
}


void MainWindow::onTimeout()
{
    mFPSLabel->setText(QString("FPS: ") + QString::number(mTetrisWidget1->getFPS()));
}
