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
    for (std::size_t idx = 0; idx < EnumInfo<PlayerType>::size(); ++idx)
    {
        std::cout << "Enum entry name: " << EnumInfo<PlayerType>::names()[idx] << std::endl;
        std::cout << "Enum entry value: " << EnumInfo<PlayerType>::values()[idx] << std::endl;
    }
    std::cout << std::endl;

    std::cout << EnumValueInfo<PlayerType, Human>::name() << std::endl;
    std::cout << EnumValueInfo<PlayerType, Human>::value() << std::endl;
    std::cout << EnumValueInfo<PlayerType, Computer>::name() << std::endl;
    std::cout << EnumValueInfo<PlayerType, Computer>::value() << std::endl;
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
