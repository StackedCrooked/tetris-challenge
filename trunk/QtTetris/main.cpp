#include <QtGui/QApplication>
#include "MainWindow.h"
#include "Futile/MainThread.h"


using Futile::MainThread;


int Tetris_RowCount()
{
    return 20;
}


int Tetris_ColumnCount()
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
    a.setApplicationName("QtTetris");
    a.setApplicationVersion("0.0 alpha");
    MainThread::Initializer scopedInit;
    MainWindow w;
    w.show();

    return a.exec();
}
