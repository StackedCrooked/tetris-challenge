#include <QtGui/QApplication>
#include "Model.h"
#include "MainWindow.h"
#include "Tetris/PlayerType.h"
#include "Futile/Enum.h"
#include "Futile/MainThread.h"
#include "Futile/LeakDetector.h"
#include "Futile/Logger.h"
#include <iostream>
#include <stdexcept>


using Futile::Logger;
using Futile::LeakDetector;
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


int run(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("QtTetris");
    a.setApplicationVersion("0.0 alpha");
    MainWindow::Initializer initMainWindow;
    MainWindow::Instance().show();
    return a.exec();
}


void TestEnum()
{
    using namespace Tetris;
    for (std::size_t idx = 0; idx < Futile::EnumInfo<PlayerType>::size(); ++idx)
    {
        std::cout << "Name: " << Futile::EnumInfo<PlayerType>::names()[idx] << ", "
                  << "value: " << Futile::EnumInfo<PlayerType>::values()[idx] << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Enumerator name: "    << Futile::EnumeratorInfo<PlayerType, Human>::name()
              << ", value: "             << Futile::EnumeratorInfo<PlayerType, Human>::value() << std::endl
              << "Enumerator name: " << Futile::EnumeratorInfo<PlayerType, Computer>::name()
              << ", value: "             << Futile::EnumeratorInfo<PlayerType, Computer>::value() << std::endl;
}


int main(int argc, char *argv[])
{
    try
    {
        TestEnum();
        Logger::Initializer initLogger;
        LeakDetector::Initializer initLeakDetector;
        MainThread::Initializer initMainThread;
        Tetris::Model::Initializer initModel;
        return run(argc, argv);
    }
    catch (const std::exception & exc)
    {
        std::cerr << "Exception caught in main: " << exc.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Anonymous exception caught in main. Exiting program." << std::endl;
    }
    return 1;
}
