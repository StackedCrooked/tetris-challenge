#include "MainWindow.h"
#include "Tetris/Assert.h"
#include "Tetris/AutoPtrSupport.h"
#include "Tetris/ComputerPlayer.h"
#include "Tetris/Game.h"
#include "Tetris/Logger.h"
#include "Tetris/Logging.h"
#include "Tetris/MakeString.h"
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
                                                        int & /*outWorkerCount*/)
    {
        int firstRow = inGameState.firstOccupiedRow();
        if (firstRow >= 15)
        {
            outSearchDepth = 8;
            outSearchWidth = 5;
            return CreatePoly<Evaluator, MakeTetrises>();
        }
        else if (firstRow >= 10)
        {
            outSearchDepth = 6;
            outSearchWidth = 6;
            return CreatePoly<Evaluator, Balanced>();
        }
        else
        {
            outSearchDepth = 5;
            outSearchWidth = 5;
            return CreatePoly<Evaluator, Survival>();
        }
    }


    void reset(const PlayerTypes & inPlayerTypes, size_t inRowCount, size_t inColCount)
    {
        mMultiplayerGame.reset();
        mMultiplayerGame.reset(new MultiplayerGame);
        for (PlayerTypes::size_type idx = 0; idx < inPlayerTypes.size(); ++idx)
        {
            Player * player = mMultiplayerGame->join(
                Create<Player>(inPlayerTypes[idx],
                               TeamName(GetTeamName(inPlayerTypes[idx], idx + 1)),
                               PlayerName(GetPlayerName(inPlayerTypes[idx], idx/2)),
                               inRowCount,
                               inColCount));
            if (inPlayerTypes[idx] == PlayerType_Computer)
            {
                player->simpleGame()->setAITweaker(&Model::Instance());
                player->simpleGame()->setStartingLevel(14);
                player->simpleGame()->setComputerMoveSpeed(50);
            }
        }
    }

private:
    Model() :
        mStartingLevelHuman(0),
        mStartingLevelComputer(0)
    {
    }

    static std::string GetTeamName(PlayerType inPlayerType, size_t inIndex)
    {
        std::stringstream ss;
        if (inPlayerType == PlayerType_Human)
        {
            ss << "Pirates";
        }
        else
        {
            ss << "Marines";
        }
         ss << " " << inIndex;
         return ss.str();
    }

    static std::string GetPlayerName(PlayerType inPlayerType, size_t inIndex)
    {
        typedef const char * CharPtr;

        static const CharPtr cPirates[] = {
            "Luffy",
            "Zoro",
            "Nami",
            "Sanji"
        };
        static const int cPirateCount = sizeof(cPirates) / sizeof(cPirates[0]);
        static int cPirateIndex = 0;

        static const CharPtr cMarines[] = {
            "Smoker",
            "Tashigi",
            "Fullbody",
            "Hina"
        };
        static const int cMarineCount = sizeof(cMarines) / sizeof(cMarines[0]);
        static int cMarineIndex = 0;

        if (inPlayerType == PlayerType_Human)
        {
            return cPirates[cPirateIndex++ % cPirateCount];
        }
        else
        {
            return cMarines[cMarineIndex++ % cMarineCount] + std::string(" (Computer)");
        }
    }

    Model(const Model &);
    Model& operator=(const Model&);

    boost::scoped_ptr<Tetris::MultiplayerGame> mMultiplayerGame;
    int mStartingLevelHuman;
    int mStartingLevelComputer;
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
    mShowLog(true)
{
    sInstance = this;

    setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));

    setWindowTitle("Piratris");


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


    QHBoxLayout * mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(theCentralWidget, 0);
    setLayout(mainLayout);

    setCentralWidget(theCentralWidget);
    onNewComputerVsComputerGame();
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
    for (TetrisWidgets::size_type idx = mTetrisWidgetHolder->children().size(); idx < inPlayerTypes.size(); ++idx)
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








