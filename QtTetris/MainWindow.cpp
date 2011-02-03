#include "MainWindow.h"
#include "Tetris/Game.h"
#include "Tetris/Logger.h"
#include <QLayout>
#include <QLabel>
#include <algorithm>
#include <iostream>


using namespace Tetris;


typedef boost::shared_ptr<Tetris::SimpleGame> SimpleGamePtr;


enum
{
    cPlayerCount = 2
};


class Model
{
public:
    static Model & Instance()
    {
        static Model fModel;
        return fModel;
    }

    std::vector<SimpleGamePtr> mTetrisGames;
    Tetris::MultiplayerGame mMultiplayerGame;


private:
    Model() :
        mTetrisGames(cPlayerCount),
        mMultiplayerGame()
    {
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
    mSwitchButton(0),
    mRestartButton(0),
    mLogField(0)
{
    sInstance = this;

    for (size_t idx = 0; idx < cPlayerCount; ++idx)
    {
        mTetrisWidgets.push_back(new TetrisWidget(this, cSquareWidth, cSquareHeight));
        mTetrisWidgets.back()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    mSwitchButton = new QPushButton("Switch", this);
    connect(mSwitchButton, SIGNAL(clicked()), this, SLOT(onRestart()));

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
    vbox->addWidget(mSwitchButton);
    vbox->addWidget(mRestartButton);
    vbox->addWidget(new QLabel("Press 'c' to clear the game."), 0);
    vbox->addWidget(mLogField, 1);


    Logger::Instance().setLogHandler(boost::bind(&MainWindow::logMessage, this, _1));

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


void MainWindow::restart()
{
    assert(Model::Instance().mTetrisGames.size() == mTetrisWidgets.size());
    std::vector<SimpleGamePtr> & theTetrisGames = Model::Instance().mTetrisGames;

    for (size_t idx = 0; idx < theTetrisGames.size(); ++idx)
    {
        SimpleGamePtr oldSimpleGamePtr(theTetrisGames[idx]);
        if (oldSimpleGamePtr)
        {
            SimpleGame & oldSimpleGame(*oldSimpleGamePtr);
            ThreadSafe<Game> threadSafeGame(oldSimpleGame.game());
            Model::Instance().mMultiplayerGame.leave(threadSafeGame);
        }

        // Forces unregistration of the event handlers.
        mTetrisWidgets[idx]->setGame(0);

        // Make sure the previous game has been deleted before creating a new one.
        theTetrisGames[idx].reset();

        SimpleGamePtr simpleGamePtr(new SimpleGame(cRowCount, cColumnCount));
        theTetrisGames[idx] = simpleGamePtr;
        Model::Instance().mMultiplayerGame.join(simpleGamePtr->game());
        mTetrisWidgets[idx]->setGame(simpleGamePtr.get());
    }
}
