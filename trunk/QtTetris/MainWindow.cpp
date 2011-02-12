#include "MainWindow.h"
#include "Tetris/Assert.h"
#include "Tetris/AutoPtrSupport.h"
#include "Tetris/Game.h"
#include "Tetris/Logger.h"
#include "Tetris/Logging.h"
#include "Tetris/MakeString.h"
#include <QLayout>
#include <QLabel>
#include <algorithm>
#include <iostream>


int Tetris_RowCount();
int Tetris_ColumnCount();
int Tetris_GetSquareWidth();
int Tetris_GetSquareHeight();


typedef boost::shared_ptr<Tetris::SimpleGame> SimpleGamePtr;


using namespace Tetris;


enum
{
    cPlayerCount = 1
};


class Model
{
public:
    static Model & Instance()
    {
        static Model fModel;
        return fModel;
    }

    Player * getPlayer(size_t inIndex)
    {
        return mMultiplayerGame->getPlayer(inIndex);
    }

    const MultiplayerGame & multiplayerGame() const
    {
        return *mMultiplayerGame;
    }

    MultiplayerGame & multiplayerGame()
    {
        return *mMultiplayerGame;
    }

    void reset(size_t inRowCount, size_t inColCount)
    {
        LogInfo(MakeString() << "RESET!!!");
        mMultiplayerGame.reset();
        mMultiplayerGame.reset(new MultiplayerGame);
        for (size_t idx = 0; idx < cPlayerCount; ++idx)
        {
            LogInfo(MakeString() << "Adding player " << idx);
            mMultiplayerGame->join(
                Create<Player>(PlayerType_Computer,
                               TeamName(GetTeamName(idx)),
                               PlayerName(GetPlayerName(idx)),
                               inRowCount,
                               inColCount
                               ));
        }
    }

private:
    Model()
    {
        LogInfo(__PRETTY_FUNCTION__);
    }

    static std::string GetTeamName(size_t inIndex)
    {
        return std::string(inIndex < cPlayerCount/2 ? "A" : "B");
    }

    static std::string GetPlayerName(size_t inIndex)
    {
        return std::string(MakeString() << (inIndex < cPlayerCount/2 ? "A" : "B") << inIndex);
    }

    Model(const Model &);
    Model& operator=(const Model&);

    boost::scoped_ptr<Tetris::MultiplayerGame> mMultiplayerGame;
};


MainWindow * MainWindow::sInstance(0);


MainWindow * MainWindow::GetInstance()
{
    return sInstance;
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mTetrisWidgets(),
    mRestartButton(0),
    mPauseButton(0),
    mLogField(0)
{
    sInstance = this;


    QWidget * theCentralWidget(new QWidget);
    for (size_t idx = 0; idx < cPlayerCount; ++idx)
    {
        mTetrisWidgets.push_back(new TetrisWidget(theCentralWidget, Tetris_GetSquareWidth(), Tetris_GetSquareHeight()));
        mTetrisWidgets.back()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    QAction * newGame = menuBar()->addMenu("Tetris")->addAction("New");
    connect(newGame, SIGNAL(triggered()), this, SLOT(onRestart()));

    mRestartButton = new QPushButton("Restart", theCentralWidget);
    connect(mRestartButton, SIGNAL(clicked()), this, SLOT(onRestart()));

    mPauseButton = new QPushButton("Pause", theCentralWidget);
    connect(mPauseButton, SIGNAL(clicked()), this, SLOT(onPaused()));

    mLogField = new QTextEdit(theCentralWidget);
    mLogField->setReadOnly(true);


    QVBoxLayout * vbox = new QVBoxLayout(theCentralWidget);
    QHBoxLayout * hbox = new QHBoxLayout;
    for (size_t idx = 0; idx < cPlayerCount; ++idx)
    {
        hbox->addWidget(mTetrisWidgets[idx]);
    }
    vbox->addItem(hbox);

    QHBoxLayout * buttonsHBox = new QHBoxLayout;
    buttonsHBox->addWidget(mPauseButton);
    buttonsHBox->addWidget(mRestartButton);

    vbox->addItem(buttonsHBox);
    vbox->addWidget(new QLabel("Press 'c' to clear the game."), 0);
    vbox->addWidget(mLogField, 1);

    setCentralWidget(theCentralWidget);

    Logger::Instance().setLogHandler(boost::bind(&MainWindow::logMessage, this, _1));
    LogInfo("Test Logger.");
    restart();
}


MainWindow::~MainWindow()
{
    typedef boost::function<void(const std::string &)> DummyFunction;
    DummyFunction dummy;
    Logger::Instance().setLogHandler(dummy);
    sInstance = 0;
}


void MainWindow::logMessage(const std::string & inMessage)
{
    mLogField->append(inMessage.c_str());
}


bool MainWindow::event(QEvent * inEvent)
{
    if (inEvent->type() == QEvent::Paint)
    {
        Logger::Instance().flush();
    }
    return QWidget::event(inEvent);
}


void MainWindow::onRestart()
{
    restart();
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


void MainWindow::restart()
{
    LogInfo(__PRETTY_FUNCTION__);
    Model::Instance().reset(Tetris_RowCount(), Tetris_ColumnCount());
    MultiplayerGame & mgame = Model::Instance().multiplayerGame();
    LogInfo(MakeString() << "Player count: " << mgame.playerCount());
    LogInfo(MakeString() << "mTetrisWidgets.size(): " << mTetrisWidgets.size());
    Assert(mgame.playerCount() == mTetrisWidgets.size());

    for (size_t idx = 0; idx < mgame.playerCount(); ++idx)
    {
        LogInfo(MakeString() << "Setting player " << idx);
        Player * player = mgame.getPlayer(idx);
        player->simpleGame()->setLevel(6);
        mTetrisWidgets[idx]->setGame(player->simpleGame());
        player->simpleGame()->applyLinePenalty(10);
    }

}








