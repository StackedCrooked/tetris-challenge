#include "MainWindow.h"
#include "NewGameDialog.h"
#include "Tetris/BlockMover.h"
#include "Tetris/ComputerPlayer.h"
#include "Tetris/Game.h"
#include "Poco/Path.h"
#include "Tetris/Utilities.h"
#include "Futile/Assert.h"
#include "Futile/AutoPtrSupport.h"
#include "Futile/Boost.h"
#include "Futile/Logger.h"
#include "Futile/Logging.h"
#include "Futile/MakeString.h"
#include "Poco/Environment.h"
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtCore/QTimer>
#include <boost/noncopyable.hpp>
#include <algorithm>
#include <iostream>
#include <sstream>


using Futile::Create;
using Futile::CreatePoly;
using Futile::LogError;
using Futile::LogInfo;
using Futile::Logger;
using Futile::MakeString;
using Futile::Str;
using Futile::Singleton;


using namespace Tetris;


int Tetris_RowCount();
int Tetris_ColumnCount();
int Tetris_GetSquareWidth();
int Tetris_GetSquareHeight();


typedef Futile::Boost::shared_ptr<SimpleGame> SimpleGamePtr;


void MainWindow::onTimerEvent()
{
    try
    {
        timerEvent();
    }
    catch (const std::exception & exc)
    {
        LogError(exc.what());
    }
}


void MainWindow::timerEvent()
{
    // We already know about the GameOver event.
    // No longer interested.
    if (mGameOver)
    {
        return;
    }


    if (Model::Instance().IsGameOver())
    {
        mGameOver = true;

        MultiplayerGame & mgame = Model::Instance().multiplayerGame();
        if (mgame.playerCount() == 1)
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle("QtTetris");
            std::string text = "And the winner is: Gravity!";
            msgBox.setText(text.c_str());
            msgBox.exec();
            return;
        }


        for (size_t idx = 0; idx < mgame.playerCount(); ++idx)
        {
            Player * player(mgame.getPlayer(idx));
            if (!player->simpleGame()->isGameOver())
            {
                player->simpleGame()->setPaused(true);
                QMessageBox msgBox;
                msgBox.setWindowTitle("QtTetris");
                std::string text = "And the winner is: " + player->playerName() + "!";
                msgBox.setText(text.c_str());
                msgBox.exec();
                return;
            }
        }
    }

}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mTetrisWidgetHolder(0),
    mTetrisWidgets(),
    mSpacing(12),
    mLogField(0),
    mShowLog(true),
    mGameOver(false)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum));
    setWindowTitle("QtTetris");

    QMenu * tetrisMenu = menuBar()->addMenu("Tetris");
    QAction * newGame = tetrisMenu->addAction("New");
    newGame->setShortcut(QKeySequence("Ctrl+N"));

    connect(newGame, SIGNAL(triggered()), this, SLOT(onNew()));

    QAction * pauseGame = tetrisMenu->addAction("Pause");
    connect(pauseGame, SIGNAL(triggered()), this, SLOT(onPaused()));
    pauseGame->setShortcut(QKeySequence("Ctrl+P"));

    QWidget * theCentralWidget(new QWidget);
    setCentralWidget(theCentralWidget);

    QVBoxLayout * vbox = new QVBoxLayout(theCentralWidget);
    mTetrisWidgetHolder = new QHBoxLayout;
    vbox->addItem(mTetrisWidgetHolder);
    vbox->setAlignment(mTetrisWidgetHolder, Qt::AlignHCenter);

    const std::string cCtrlKey = (Poco::Environment::osName() == "Darwin") ? "Command-N" : "Ctrl-N";
    vbox->addWidget(new QLabel(std::string("Press " + cCtrlKey + " to start a new game.").c_str()));
    mTetrisWidgetHolder->setSpacing(mSpacing);
    while (mTetrisWidgets.size() < 2)
    {
        mTetrisWidgets.push_back(new TetrisWidget(centralWidget(), Tetris_GetSquareWidth(), Tetris_GetSquareHeight()));
        mTetrisWidgetHolder->addWidget(mTetrisWidgets.back(), 0);
    }

    if (mShowLog)
    {
        mLogField = new QTextEdit(theCentralWidget);
        mLogField->setReadOnly(true);
        vbox->addWidget(mLogField, 1);
        Logger::Instance().setLogHandler(boost::bind(&MainWindow::logMessage, this, _1));
        LogInfo(Poco::Path::current());
    }

    QTimer * timer(new QTimer(this));
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimerEvent()));
    timer->start(500);

    onNewComputerVsComputerGame();
}


MainWindow::~MainWindow()
{
}


void MainWindow::logMessage(const std::string & inMessage)
{
    if (mLogField)
    {
        mLogField->append(inMessage.c_str());
    }
}


void MainWindow::closeEvent(QCloseEvent*)
{
}


bool MainWindow::event(QEvent * inEvent)
{
    if (inEvent->type() == QEvent::Paint)
    {
        Logger::Instance().flush();
    }
    return QWidget::event(inEvent);
}


void MainWindow::onNewGame(const PlayerTypes & inPlayerTypes)
{
    if (inPlayerTypes.size() > mTetrisWidgets.size())
    {
        throw std::runtime_error(Str() << "Too make players: " << inPlayerTypes.size() << ". View only supports " << mTetrisWidgets.size() << " players.");
    }

    // Unset the GameOver state.
    mGameOver = false;

    // Unset all widgets.
    for (TetrisWidgets::size_type idx = 0; idx < mTetrisWidgets.size(); ++idx)
    {
        TetrisWidget * tetrisWidget = mTetrisWidgets[idx];
        tetrisWidget->setPlayer(NULL);
        tetrisWidget->releaseKeyboard();
        tetrisWidget->hide();
    }

    // Reset the game.
    Model::Instance().newGame(inPlayerTypes, Tetris_RowCount(), Tetris_ColumnCount());

    // Add the players.
    MultiplayerGame & mgame = Model::Instance().multiplayerGame();
    for (size_t idx = 0; idx < mgame.playerCount(); ++idx)
    {
        Player * player = mgame.getPlayer(idx);
        TetrisWidget * tetrisWidget = mTetrisWidgets[idx];
        tetrisWidget->setPlayer(player);
        tetrisWidget->show();
        if (player->type() == Human)
        {
            tetrisWidget->setFocus();
        }
    }
}


void MainWindow::onNew()
{
    NewGameDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        NewGameDialog::Selection selection = dialog.selection();
        if (selection == NewGameDialog::Selection_Human)
        {
            onNewSingleHumanPlayerGame();
        }
        else if (selection == NewGameDialog::Selection_Computer)
        {
            onNewSingleComputerPlayerGame();
        }
        else if (selection == NewGameDialog::Selection_HumanVsComputer)
        {
            onNewHumanVsComputerGame();
        }
        else if (selection == NewGameDialog::Selection_ComputerVsComputer)
        {
            onNewComputerVsComputerGame();
        }
    }
    updateGeometry();
}


void MainWindow::onNewSingleHumanPlayerGame()
{
    PlayerTypes playerTypes;
    playerTypes.push_back(Human);
    onNewGame(playerTypes);
}


void MainWindow::onNewSingleComputerPlayerGame()
{
    PlayerTypes playerTypes;
    playerTypes.push_back(Computer);
    onNewGame(playerTypes);
}


void MainWindow::onNewHumanVsComputerGame()
{
    PlayerTypes playerTypes;
    playerTypes.push_back(Human);
    playerTypes.push_back(Computer);
    onNewGame(playerTypes);
}


void MainWindow::onNewComputerVsComputerGame()
{
    PlayerTypes playerTypes;
    playerTypes.push_back(Computer);
    playerTypes.push_back(Computer);
    onNewGame(playerTypes);
}


void MainWindow::on2v2Game()
{
    PlayerTypes playerTypes;
    playerTypes.push_back(Human);
    playerTypes.push_back(Computer);
    playerTypes.push_back(Computer);
    playerTypes.push_back(Computer);
    onNewGame(playerTypes);

}


void MainWindow::onPaused()
{
    MultiplayerGame & mgame = Model::Instance().multiplayerGame();
    for (size_t idx = 0; idx < mgame.playerCount(); ++idx)
    {
        Player * player = mgame.getPlayer(idx);
        bool isPaused = player->simpleGame()->isPaused();
        player->simpleGame()->setPaused(!isPaused);
    }
}


void MainWindow::onPenalty()
{
    MultiplayerGame & mgame = Model::Instance().multiplayerGame();
    for (size_t idx = 0; idx < mgame.playerCount(); ++idx)
    {
        Player * player = mgame.getPlayer(idx);
        player->simpleGame()->applyLinePenalty(4);
        break;
    }
}