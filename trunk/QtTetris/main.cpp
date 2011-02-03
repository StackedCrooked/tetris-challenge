#include <QtGui/QApplication>
#include "MainWindow.h"
#include "Tetris/MainThread.h"


int TetrisWidget_NumRows()
{
    return 20;
}


int TetrisWidget_NumColumns()
{
    return 10;
}


int Tetris_GetSquareWidth()
{
    return 20;
}


int Tetris_GetSquareHeight()
{
    return 20;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Tetris::MainThread::Initializer scopedInit;
    MainWindow w;
    w.show();

    return a.exec();
}
