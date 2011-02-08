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


using namespace Tetris;


typedef boost::shared_ptr<Tetris::SimpleGame> SimpleGamePtr;


enum
{
    cPlayerCount = 4
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

    void reset()
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
                               PlayerName(GetPlayerName(idx))));
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


//
// Configuration
//
const int cRowCount(20);
const int cColumnCount(10);
const int cSquareWidth(20);
const int cSquareHeight(20);


MainWindow * MainWindow::sInstance(0);


MainWindow * MainWindow::GetInstance()
{
    return sInstance;
}


MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    mTetrisWidgets(),
    mRestartButton(0),
    mLogField(0),
    mRestartFlag(false)
{
    sInstance = this;

    for (size_t idx = 0; idx < cPlayerCount; ++idx)
    {
        mTetrisWidgets.push_back(new TetrisWidget(this, cSquareWidth, cSquareHeight));
        mTetrisWidgets.back()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    mRestartButton = new QPushButton("Restart", this);
    connect(mRestartButton, SIGNAL(clicked()), this, SLOT(onRestart()));

    mLogField = new QTextEdit(this);
    mLogField->setReadOnly(true);


    QVBoxLayout * vbox = new QVBoxLayout(this);
    QHBoxLayout * hbox = new QHBoxLayout;
    for (size_t idx = 0; idx < cPlayerCount; ++idx)
    {
        hbox->addWidget(mTetrisWidgets[idx]);
    }
    vbox->addItem(hbox);
    vbox->addWidget(mRestartButton);
    vbox->addWidget(new QLabel("Press 'c' to clear the game."), 0);
    vbox->addWidget(mLogField, 1);


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
    if (mRestartFlag)
    {
        throw std::logic_error("MainWindow::onRestart: mRestartFlag is already true!");
    }
    mRestartFlag = true;
    InvokeLater(boost::bind(&MainWindow::restart, this));
}


void MainWindow::restart()
{
    LogInfo(__PRETTY_FUNCTION__);
    Model::Instance().reset();
    MultiplayerGame & mgame = Model::Instance().multiplayerGame();
    LogInfo(MakeString() << "Player count: " << mgame.playerCount());
    LogInfo(MakeString() << "mTetrisWidgets.size(): " << mTetrisWidgets.size());
    Assert(mgame.playerCount() == mTetrisWidgets.size());

    for (size_t idx = 0; idx < mgame.playerCount(); ++idx)
    {
        LogInfo(MakeString() << "Setting player " << idx);
        Player * player = mgame.getPlayer(idx);
        mTetrisWidgets[idx]->setGame(player->simpleGame());
    }

    // Ok, we're done.
    mRestartFlag = false;
}








