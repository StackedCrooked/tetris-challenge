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


enum Error
{
    Error_None,
    Error_MainScope,
    Error_ModelScope,
    Error_ViewScope
};


// EnterViewScope creates the view scope.
int EnterViewScope(int argc, char *argv[], Model & model)
{
    int result = Error_ViewScope;
    try
    {
        QApplication a(argc, argv);
        a.setApplicationName("QtTetris");
        a.setApplicationVersion("0.0 alpha");
        a.setActiveWindow(new MainWindow(NULL, model));
        a.activeWindow()->show();
        result = a.exec();
    }
    catch (const std::exception & exc)
    {
        std::cerr << "Exception caught from Qt application: " << exc.what() << std::endl;
    }
    return result;
}


// EnterModelScope creates the model scope.
int EnterModelScope(int argc, char * argv[])
{
    int result = Error_ModelScope;    
    try
    {
        Model model;
        int err = EnterViewScope(argc, argv, model);
        model.quit();
        result = err;
    }
    catch (const std::exception & exc)
    {
        std::cerr << "Exception caught from Tetris application: " << exc.what() << std::endl;
    }
    return result;
}


int main(int argc, char *argv[])
{
    int result = Error_MainScope;
    try
    {
        // Define the lifetime scope for the singleton objects.
        Logger::ScopedInitializer initLogger;
        LeakDetector::ScopedInitializer initLeakDetector;
        MainThread::ScopedInitializer initMainThread;

        // Enter application scope.
        result = EnterModelScope(argc, argv);
    }
    catch (const std::exception & exc)
    {
        std::cerr << "Exception caught in main: " << exc.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Anonymous exception caught in main. Exiting program." << std::endl;
    }
    return result;
}
