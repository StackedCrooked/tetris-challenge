#define QT_NO_KEYWORDS
#define QT_NO_STL


#include <QtGui/QApplication>
#include "MainWindow.h"
#include "Model.h"
#include "Tetris/PlayerType.h"
#include "Tetris/Unicode.h"
#include "Futile/LeakDetector.h"
#include "Futile/Logger.h"
#include "Futile/MainThread.h"
#include "Futile/MakeString.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>


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


namespace {


void Print(const std::string & msg)
{
    std::cout << msg << std::endl;
}


} // anonymous namespace


int RunGUI(int argc, char *argv[], Model & model)
{
    Futile::Logger::Instance().addLogHandler(boost::bind(&Print, _1));

    QApplication app(argc, argv);
    app.setApplicationName("QtTetris");
    app.setApplicationVersion("0.0 alpha");
    try
    {
        app.setActiveWindow(new MainWindow(NULL, model));
        app.activeWindow()->show();
        return app.exec();
    }
    catch (const std::exception & exc)
    {
        std::string message(Futile::SS() << "Caught exception: " << exc.what());
        QMessageBox::critical(NULL, "QtTetris", message.c_str(), QMessageBox::Ok, QMessageBox::NoButton);
    }
    catch (...)
    {
        QMessageBox::critical(NULL, "QtTetris", "Caught anonymous exception. Exiting.", QMessageBox::Ok, QMessageBox::NoButton);
    }
    return 1;
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
    catch (const std::exception & exc)
    {
        std::cerr << "QtTetris: Caught exception: " << exc.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "QtTetris: Caught anonymous exception." << std::endl;
    }
    return 1;
}
