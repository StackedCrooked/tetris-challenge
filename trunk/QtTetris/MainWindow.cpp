#include "MainWindow.h"
#include <QLayout>
#include <QLabel>
#include "Tetris/AutoPtrSupport.h"
#include "Tetris/Block.h"
#include "Tetris/Game.h"
#include "Tetris/Threading.h"
#include "Tetris/Grid.h"
#include <iostream>


using namespace Tetris;


static Protected<Game> Tetris_CreateGame()
{
    return Protected<Game>(Create<Game>(TetrisWidget_NumRows(),
                                        TetrisWidget_NumColumns()));
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mTetrisWidget(0)
{
    QWidget * centralWidget = new QWidget(this);

    mTetrisWidget = new TetrisWidget(centralWidget, Tetris_CreateGame());

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(new QLabel("before"), 1, Qt::AlignCenter);
    hbox->addWidget(mTetrisWidget, 2, Qt::AlignCenter);
    hbox->addWidget(new QLabel("after"), 1, Qt::AlignCenter);

    centralWidget->setLayout(hbox);
    setCentralWidget(centralWidget);
}


MainWindow::~MainWindow()
{
}
