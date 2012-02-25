#include "MainWindow.h"
#include "NewGameDialog.h"
#include "Tetris/ComputerPlayer.h"
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
#include <iomanip>
#include <iostream>
#include <sstream>


int Tetris_RowCount();
int Tetris_ColumnCount();
int Tetris_GetSquareWidth();
int Tetris_GetSquareHeight();


namespace QtTetris {


using namespace Futile;


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
}


MainWindow::MainWindow(QWidget *parent, Model & inModel) :
    QMainWindow(parent),
    mModel(inModel),
    mTetrisWidgetHolder(0),
    mTetrisWidgets(),
    mSpacing(12),
    mLogField(0),
    mShowLog(false),
    mGameOver(false)
{
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum));
    setWindowTitle("QtTetris");

    QMenu * tetrisMenu = menuBar()->addMenu("&Tetris");

    QAction * newGame = tetrisMenu->addAction("&New");
    newGame->setShortcut(QKeySequence("Ctrl+N"));
    connect(newGame, SIGNAL(triggered()), this, SLOT(onNew()));

    QAction * pauseGame = tetrisMenu->addAction("&Pause");
    pauseGame->setShortcut(QKeySequence("Ctrl+P"));
    connect(pauseGame, SIGNAL(triggered()), this, SLOT(onPaused()));

    QAction * quitGame = tetrisMenu->addAction("E&xit");
    connect(quitGame, SIGNAL(triggered()), this, SLOT(onExit()));

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

    onNewSingleComputerPlayerGame();
}


MainWindow::~MainWindow()
{
}


void MainWindow::logMessage(const std::string & inMessage)
{
    static UInt64 fBegin = GetCurrentTimeMs();
    UInt64 time = GetCurrentTimeMs() - fBegin;
    std::cout << std::setw(8) << std::setfill('0') << unsigned(time) << " " << inMessage << std::endl;
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
        throw std::runtime_error(SS() << "Too make players: " << inPlayerTypes.size() << ". View only supports " << mTetrisWidgets.size() << " players.");
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
    mModel.newGame(inPlayerTypes, Tetris_RowCount(), Tetris_ColumnCount());

    // Add the players.
    MultiplayerGame & mgame = mModel.multiplayerGame();
    for (std::size_t idx = 0; idx < mgame.playerCount(); ++idx)
    {
        Player * player = mgame.getPlayer(idx);
        TetrisWidget * tetrisWidget = mTetrisWidgets[idx];
        tetrisWidget->setPlayer(player);
        tetrisWidget->show();
        if (player->type() == PlayerType_Human)
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
    playerTypes.push_back(PlayerType_Human);
    onNewGame(playerTypes);
}


void MainWindow::onNewSingleComputerPlayerGame()
{
    PlayerTypes playerTypes;
    playerTypes.push_back(PlayerType_Computer);
    onNewGame(playerTypes);
}


void MainWindow::onNewHumanVsComputerGame()
{
    PlayerTypes playerTypes;
    playerTypes.push_back(PlayerType_Human);
    playerTypes.push_back(PlayerType_Computer);
    onNewGame(playerTypes);
}


void MainWindow::onNewComputerVsComputerGame()
{
    PlayerTypes playerTypes;
    playerTypes.push_back(PlayerType_Computer);
    playerTypes.push_back(PlayerType_Computer);
    onNewGame(playerTypes);
}


void MainWindow::on2v2Game()
{
    PlayerTypes playerTypes;
    playerTypes.push_back(PlayerType_Human);
    playerTypes.push_back(PlayerType_Computer);
    playerTypes.push_back(PlayerType_Computer);
    playerTypes.push_back(PlayerType_Computer);
    onNewGame(playerTypes);

}


void MainWindow::onPaused()
{
    MultiplayerGame & mgame = mModel.multiplayerGame();
    for (std::size_t idx = 0; idx < mgame.playerCount(); ++idx)
    {
        Player * player = mgame.getPlayer(idx);
        bool isPaused = player->game().isPaused();
        player->game().setPaused(!isPaused);
    }
}


void MainWindow::onPenalty()
{
    MultiplayerGame & mgame = mModel.multiplayerGame();
    for (std::size_t idx = 0; idx < mgame.playerCount(); ++idx)
    {
        Player * player = mgame.getPlayer(idx);
        player->game().applyLinePenalty(4);
        break;
    }
}


void MainWindow::onExit()
{
    close();
}


} // namespace QtTetris
