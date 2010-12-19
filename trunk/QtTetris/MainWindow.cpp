#include "MainWindow.h"
#include <QLayout>
#include <QLabel>
#include <iostream>


using namespace Tetris;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    QWidget * centralWidget = new QWidget(this);

    mTetrisWidget = new TetrisWidget(centralWidget);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(mTetrisWidget, 1, Qt::AlignCenter);

    centralWidget->setLayout(hbox);
    setCentralWidget(centralWidget);
}


MainWindow::~MainWindow()
{
}
