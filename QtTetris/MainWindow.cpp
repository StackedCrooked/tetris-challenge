#include "MainWindow.h"
#include "Tetris/Assert.h"
#include "Tetris/AutoPtrSupport.h"
#include "Tetris/ComputerPlayer.h"
#include "Tetris/Game.h"
#include "Tetris/Logger.h"
#include "Tetris/Logging.h"
#include "Tetris/MakeString.h"
#include "Poco/Environment.h"
#include <QLayout>
#include <QLabel>
#include <algorithm>
#include <iostream>
#include <sstream>


int Tetris_RowCount();
int Tetris_ColumnCount();
int Tetris_GetSquareWidth();
int Tetris_GetSquareHeight();


typedef boost::shared_ptr<Tetris::SimpleGame> SimpleGamePtr;


using namespace Tetris;


class Model : public ComputerPlayer::Tweaker
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

    virtual std::auto_ptr<Evaluator> updateAIParameters(const GameState & inGameState,
                                                        int & outSearchDepth,
                                                        int & outSearchWidth,
                                                        int & outWorkerCount)
    {
        outWorkerCount = std::max<int>(1, mCPUCount / 2);
        int firstRow = inGameState.firstOccupiedRow();
        if (firstRow > 10)
        {
            outSearchDepth = 8;
            outSearchWidth = 5;
            return CreatePoly<Evaluator, MakeTetrises>();
        }
        else
        {
            outSearchDepth = 4;
            outSearchWidth = 4;
            return CreatePoly<Evaluator, Survival>();
        }
    }


    void reset(const PlayerTypes & inPlayerTypes, size_t inRowCount, size_t inColCount)
    {
        mMultiplayerGame.reset();
        mMultiplayerGame.reset(new MultiplayerGame);
        bool allComputer = true;
        for (PlayerTypes::size_type idx = 0; idx < inPlayerTypes.size(); ++idx)
        {
            if (inPlayerTypes[idx] != PlayerType_Computer)
            {
                allComputer = false;
                break;
            }
        }

        std::string teamName = "Team 1";
        for (PlayerTypes::size_type idx = 0; idx < inPlayerTypes.size(); ++idx)
        {
            if (idx >= inPlayerTypes.size() / 2)
            {
                teamName = "Team 2";
            }

            Player * player = mMultiplayerGame->join(
                Create<Player>(inPlayerTypes[idx],
                               TeamName(teamName),
                               PlayerName(GetPlayerName(inPlayerTypes[idx])),
                               inRowCount,
                               inColCount));
            if (inPlayerTypes[idx] == PlayerType_Computer)
            {
                player->simpleGame()->setAITweaker(&Model::Instance());
                player->simpleGame()->setStartingLevel(allComputer ? 9 : 0);
                player->simpleGame()->setComputerMoveSpeed(allComputer ? 100 : 5);
            }
        }
    }

private:
    Model() :
        mNames(),
        mNamesIndex(0),
        mHumanName(),
        mCPUCount(Poco::Environment::processorCount())
    {
        mNames.push_back("Zoro");
        mNames.push_back("Nami");
        mNames.push_back("Sanji");
        mNames.push_back("Vivi");
    }

    std::string GetHumanPlayerName()
    {
        bool ok = false;
        QString text = QInputDialog::getText(NULL,
                                             "QtTetris",
                                             "Player name:",
                                             QLineEdit::Normal,
                                             QString(),
                                             &ok);
        if (ok)
        {
            return text.toUtf8().data();
        }
        else
        {
            QMessageBox::information(NULL, "QtTetris", "Your name shall be: Luffy!", QMessageBox::Ok);
            return "Luffy";
        }
    }

    std::string GetPlayerName(PlayerType inPlayerType)
    {
        if (inPlayerType == PlayerType_Human)
        {
            if (mHumanName.empty())
            {
                mHumanName = GetHumanPlayerName();
            }
            return mHumanName;
        }
        else
        {
            mNamesIndex = (mNamesIndex + 1) % mNames.size();
            return mNames[mNamesIndex];
        }
    }

    Model(const Model &);
    Model& operator=(const Model&);

    boost::scoped_ptr<Tetris::MultiplayerGame> mMultiplayerGame;

    typedef std::vector<std::string> Names;
    Names mNames;
    Names::size_type mNamesIndex;

    std::string mHumanName;

    int mCPUCount;
};


MainWindow * MainWindow::sInstance(0);


MainWindow * MainWindow::GetInstance()
{
    return sInstance;
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mTetrisWidgetHolder(0),
    mTetrisWidgets(),
    mSpacing(12),
    mLogField(0),
    mShowLog(false)
{
    sInstance = this;

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum));

    setWindowTitle("QtTetris");

    QToolBar * toolBar = addToolBar("Tetris");
    toolBar->setMovable(false);
    QAction * pauseAction = toolBar->addAction("Pause");
    connect(pauseAction, SIGNAL(triggered()), this, SLOT(onPaused()));

    QAction * tbNewSinglePlayerGame = toolBar->addAction("Single Player");
    connect(tbNewSinglePlayerGame, SIGNAL(triggered()), this, SLOT(onNewSinglePlayerGame()));

    QAction * tbNewHumanVsComputerGame = toolBar->addAction("Human vs Computer");
    connect(tbNewHumanVsComputerGame, SIGNAL(triggered()), this, SLOT(onNewHumanVsComputerGame()));

    QAction * tbNewComputerVsComputerGame = toolBar->addAction("Computer vs Computer");
    connect(tbNewComputerVsComputerGame, SIGNAL(triggered()), this, SLOT(onNewComputerVsComputerGame()));

//    QAction * tbNewHumanComputervsComputerComputerGame = toolBar->addAction("2v2");
//    connect(tbNewHumanComputervsComputerComputerGame, SIGNAL(triggered()), this, SLOT(on2v2Game()));


    QWidget * theCentralWidget(new QWidget);

    QMenu * tetrisMenu = menuBar()->addMenu("Tetris");

    QAction * newSinglePlayerGame = tetrisMenu->addAction("New Single Player Game");
    connect(newSinglePlayerGame, SIGNAL(triggered()), this, SLOT(onNewSinglePlayerGame()));

    QAction * newHumanVsComputerGame = tetrisMenu->addAction("New Human vs Computer Game");
    connect(newHumanVsComputerGame, SIGNAL(triggered()), this, SLOT(onNewHumanVsComputerGame()));

    QAction * newComputerVsComputerGame = tetrisMenu->addAction("New Computer vs Computer Game");
    connect(newComputerVsComputerGame, SIGNAL(triggered()), this, SLOT(onNewComputerVsComputerGame()));

    QAction * pauseGame = tetrisMenu->addAction("Pause");
    connect(pauseGame, SIGNAL(triggered()), this, SLOT(onPaused()));

    QVBoxLayout * vbox = new QVBoxLayout(theCentralWidget);
    mTetrisWidgetHolder = new QHBoxLayout;
    vbox->addItem(mTetrisWidgetHolder);
    vbox->setAlignment(mTetrisWidgetHolder, Qt::AlignHCenter);
    mTetrisWidgetHolder->setSpacing(mSpacing);

    if (mShowLog)
    {
        mLogField = new QTextEdit(theCentralWidget);
        mLogField->setReadOnly(true);
        vbox->addWidget(mLogField, 1);
        Logger::Instance().setLogHandler(boost::bind(&MainWindow::logMessage, this, _1));
    }

    setCentralWidget(theCentralWidget);
    onNewHumanVsComputerGame();
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
    if (mLogField)
    {
        mLogField->append(inMessage.c_str());
    }
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
    // Unset all widgets.
    for (TetrisWidgets::size_type idx = 0; idx < mTetrisWidgets.size(); ++idx)
    {
        mTetrisWidgets[idx]->setPlayer(0);
    }

    // Ensure that we have widget for each player.
    while (mTetrisWidgets.size() < inPlayerTypes.size())
    {
        mTetrisWidgets.push_back(new TetrisWidget(centralWidget(), Tetris_GetSquareWidth(), Tetris_GetSquareHeight()));
    }

    // Add the new tetris widgets to the layout
    for (TetrisWidgets::size_type idx = mTetrisWidgetHolder->children().size(); idx < mTetrisWidgets.size(); ++idx)
    {
        mTetrisWidgetHolder->addWidget(mTetrisWidgets[idx], 0);
        mTetrisWidgets[idx]->show();
    }

    // Remove surplus widgets from layout.
    for (PlayerTypes::size_type idx = inPlayerTypes.size(); idx < mTetrisWidgets.size(); ++idx)
    {
        mTetrisWidgets[idx]->hide();
        mTetrisWidgetHolder->removeWidget(mTetrisWidgets[idx]);
    }

    // Reset the game.
    Model::Instance().reset(inPlayerTypes, Tetris_RowCount(), Tetris_ColumnCount());

    // Add the players.
    MultiplayerGame & mgame = Model::Instance().multiplayerGame();
    for (size_t idx = 0; idx < mgame.playerCount(); ++idx)
    {
        Player * player = mgame.getPlayer(idx);
        mTetrisWidgets[idx]->setPlayer(player);
    }
}


void MainWindow::onNewSinglePlayerGame()
{
    PlayerTypes playerTypes;
    playerTypes.push_back(PlayerType_Human);
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








