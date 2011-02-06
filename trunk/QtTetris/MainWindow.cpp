#include "MainWindow.h"
#include "Tetris/Assert.h"
#include "Tetris/Game.h"
#include "Tetris/Logger.h"
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

    Players mPlayers;
    Tetris::MultiplayerGame mMultiplayerGame;
private:
    Model() :
        mPlayers(),
        mMultiplayerGame()
    {
        // Assemble Team A
        for (size_t idx = 0; idx < cPlayerCount/2; ++idx)
        {
            mPlayers.push_back(Player(PlayerType_Computer, TeamName("Team A"), PlayerName(MakeString() << "A" << (idx + 1))));
        }

        // Assemble Team B
        for (size_t idx = cPlayerCount/2; idx < cPlayerCount; ++idx)
        {
            mPlayers.push_back(Player(PlayerType_Computer, TeamName("Team B"), PlayerName(MakeString() << "B" << (idx + 1))));
        }
    }

    Model(const Model &);
    Model& operator=(const Model&);
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
    mLogField(0)
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

    Players & players = Model::Instance().mPlayers;
    for (size_t idx = 0; idx < cPlayerCount; ++idx)
    {
        Player player(players[idx]);
        mTetrisWidgets[idx]->setGame(&player.simpleGame());
        Model::Instance().mMultiplayerGame.join(player);
    }
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


void MainWindow::restart()
{
    Assert(Model::Instance().mPlayers.size() == mTetrisWidgets.size());

    Players & players = Model::Instance().mPlayers;


    // Unregister all
    for (size_t idx = 0; idx < cPlayerCount; ++idx)
    {
        Player player(players[idx]);
        mTetrisWidgets[idx]->setGame(0);
        Model::Instance().mMultiplayerGame.leave(player);
        player.resetGame();
    }

    players.clear();

    // Assemble Team A
    for (size_t idx = 0; idx < cPlayerCount/2; ++idx)
    {
        players.push_back(Player(PlayerType_Computer, TeamName("Team A"), PlayerName(MakeString() << "A" << (idx + 1))));
    }

    // Assemble Team B
    for (size_t idx = cPlayerCount/2; idx < cPlayerCount; ++idx)
    {
        players.push_back(Player(PlayerType_Computer, TeamName("Team B"), PlayerName(MakeString() << "B" << (idx + 1))));
    }


    for (size_t idx = 0; idx < cPlayerCount; ++idx)
    {
        Player player(players[idx]);
        mTetrisWidgets[idx]->setGame(&player.simpleGame());
        Model::Instance().mMultiplayerGame.join(player);
    }
}
