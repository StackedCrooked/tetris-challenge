#include <QApplication>
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

    MainWindow::ScopedInitializer initMainWindow;
    MainWindow::Instance().show();

    return a.exec();
}


int main(int argc, char *argv[])
{
    try
    {
        Logger::ScopedInitializer initLogger;
        LeakDetector::ScopedInitializer initLeakDetector;
        MainThread::ScopedInitializer initMainThread;
        Tetris::Model::ScopedInitializer initModel;
        return run(argc, argv);
    }
    catch (const std::exception& exc)
    {
        std::cerr << "Exception caught in main: " << exc.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Anonymous exception caught in main. Exiting program." << std::endl;
    }
    return 1;
}
