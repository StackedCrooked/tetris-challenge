#define QT_NO_KEYWORDS


#include <QtGui/QApplication>
#include "Model.h"
#include "MainWindow.h"
#include "Tetris/PlayerType.h"
#include "Futile/MainThread.h"
#include "Futile/LeakDetector.h"
#include "Futile/Logger.h"
#include <iostream>
#include <stdexcept>


using namespace Futile;
using namespace Tetris;
using namespace QtTetris;


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


int RunGUI(int argc, char *argv[], Model& model)
{
    QApplication a(argc, argv);
    a.setApplicationName("QtTetris");
    a.setApplicationVersion("0.0 alpha");
    a.setActiveWindow(new MainWindow(NULL, model));
    a.activeWindow()->show();
    return a.exec();
}


int RunApp(int argc, char * argv[])
{
    Model model;
    return RunGUI(argc, argv, model);
}


int main(int argc, char *argv[])
{
    try
    {
        // Define the lifetime scope for the singleton objects.
        Logger::ScopedInitializer initLogger;
        LeakDetector::ScopedInitializer initLeakDetector;
        MainThread::ScopedInitializer initMainThread;

        // Enter application scope.
        return RunApp(argc, argv);
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
