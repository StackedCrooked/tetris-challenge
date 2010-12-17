#include <QtGui/QApplication>
#include "MainWindow.h"


int TetrisWidget_NumRows()
{
    return 20;
}


int TetrisWidget_NumColumns()
{
    return 10;
}


int Tetris_GetUnitWidth()
{
    return 20;
}


int Tetris_GetUnitHeight()
{
    return 20;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
