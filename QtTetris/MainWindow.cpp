#include "MainWindow.h"
#include "Tetris/Game.h"
#include <QLayout>
#include <QLabel>
#include <algorithm>
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
    mTetrisPlayers(cPlayerCount),
    mTetrisWidgets(),
    mSwitchButton(0),
    mRestartButton(0)
{

    for (size_t idx = 0; idx < cPlayerCount; ++idx)
    {
        mTetrisWidgets.push_back(new TetrisWidget(this, cSquareWidth, cSquareHeight));
        mTetrisWidgets.back()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    mSwitchButton = new QPushButton("Switch", this);
    connect(mSwitchButton, SIGNAL(clicked()), this, SLOT(onRestart()));

    mRestartButton = new QPushButton("Restart", this);
    connect(mRestartButton, SIGNAL(clicked()), this, SLOT(onRestart()));


    QVBoxLayout * vbox = new QVBoxLayout(this);
    QHBoxLayout * hbox = new QHBoxLayout;
    for (size_t idx = 0; idx < cPlayerCount; ++idx)
    {
        hbox->addWidget(mTetrisWidgets[idx]);
    }
    vbox->addItem(hbox);
    vbox->addWidget(mSwitchButton);
    vbox->addWidget(mRestartButton);
    vbox->addWidget(new QLabel("Press 'c' to clear the game."), 0);

    restart();
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
    mTetrisPlayers.clear();
    for (size_t idx = 0; idx < cPlayerCount; ++idx)
    {
        mTetrisWidgets[idx]->setGame(0);
        SimpleGamePtr simpleGamePtr(new SimpleGame(cRowCount, cColumnCount));
        mTetrisPlayers.push_back(simpleGamePtr);
        mMultiplayerGame.join(simpleGamePtr->game());
        mTetrisWidgets[idx]->setGame(simpleGamePtr.get());
    }
}
