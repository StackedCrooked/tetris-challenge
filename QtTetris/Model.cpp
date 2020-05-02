#include "Model.h"
#include "Futile/AutoPtrSupport.h"
#include "Poco/Environment.h"
#include <algorithm>
#include <chrono>


namespace Tetris {


Model::Model() :
    mNames(),
    mNamesIndex(0),
    mCPUCount(Poco::Environment::processorCount()),
    mGameOver(true),
    mRandomEngine(std::chrono::system_clock::now().time_since_epoch().count())
{
    mNames.push_back("Zoro");
    mNames.push_back("Luffy");
    mNames.push_back("Nami");
    mNames.push_back("Sanji");
    mNames.push_back("Chopper");
    std::shuffle(mNames.begin(), mNames.end(), mRandomEngine);
}


Model::~Model()
{
}


bool Model::IsGameOver()
{
    if (!mMultiplayerGame)
    {
        return true;
    }


    for (std::size_t idx = 0; idx < mMultiplayerGame->playerCount(); ++idx)
    {
        Player* player = mMultiplayerGame->getPlayer(idx);
        if (player->simpleGame()->isGameOver())
        {
            mGameOver = true;
        }
    }
    return mGameOver;
}


Player* Model::getPlayer(std::size_t inIndex)
{
    return mMultiplayerGame->getPlayer(inIndex);
}


const MultiplayerGame& Model::multiplayerGame() const
{
    return *mMultiplayerGame;
}


MultiplayerGame& Model::multiplayerGame()
{
    if (!mMultiplayerGame)
    {
        throw std::runtime_error("There is currently no game running.");
    }
    return *mMultiplayerGame;
}


const Evaluator& Model::updateAIParameters(const Player& inPlayer,
                                            int& outSearchDepth,
                                            int& outSearchWidth,
                                            int& outWorkerCount,
                                            int& outMoveSpeed,
                                            BlockMover::MoveDownBehavior& outMoveDownBehavior)
{
    if (!inPlayer.simpleGame())
    {
        throw std::runtime_error("GameState is null!");
    }

    const Game& game = *inPlayer.simpleGame();

    // This program may use all available CPUs minus one (in order to avoid hanging the system.)
    static const auto available_cpu_count = Poco::Environment::processorCount() - 1;

    const auto num_cpus_per_player = available_cpu_count / multiplayerGame().playerCount();

    outWorkerCount = std::max(1, static_cast<int>(num_cpus_per_player));

    int currentHeight = game.stats().currentHeight();

    // Drop or not?
    if (currentHeight < 10)
    {
        outMoveDownBehavior = BlockMover::MoveDownBehavior_Move;
    }
    else
    {
        outMoveDownBehavior = BlockMover::MoveDownBehavior_Drop;
    }

    if (0)
    {
        // Tactics adjustment
        if (currentHeight < 10)
        {
            outSearchDepth = 20;
            outSearchWidth = 4;
            outMoveSpeed = 60;
            return Confused::Instance();
        }
        else
        {
            outSearchDepth = 20;
            outSearchWidth = 4;
            outMoveSpeed = 40;
            return Confused::Instance();
        }
    }

    // Tactics adjustment
    if (currentHeight < 12)
    {
        outSearchDepth = 20;
        outSearchWidth = 2;
        outMoveSpeed = 60;
        return MakeTetrises::Instance();
    }
    else
    {
        outSearchDepth = 4;
        outSearchWidth = 4;
        outMoveSpeed = 60;
        return Survival::Instance();
    }
}


void Model::newGame(const PlayerTypes& inPlayerTypes, std::size_t inRowCount, std::size_t inColumnCount)
{
    mMultiplayerGame.reset();
    mMultiplayerGame.reset(new MultiplayerGame(inRowCount, inColumnCount));
    mGameOver = false;
    bool allComputer = true;
    for (PlayerTypes::size_type idx = 0; idx < inPlayerTypes.size(); ++idx)
    {
        if (inPlayerTypes[idx] != Computer)
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

        Player* player(0);
        PlayerType playerType = inPlayerTypes[idx];
        if (playerType == Human)
        {
            player = mMultiplayerGame->addHumanPlayer(TeamName(teamName),
                                                      PlayerName(GetPlayerName(inPlayerTypes[idx])));
        }
        else if (playerType == Computer)
        {
            player = mMultiplayerGame->addComputerPlayer(TeamName(teamName),
                                                         PlayerName(GetPlayerName(inPlayerTypes[idx])),
                                                         this);
            if (ComputerPlayer* computerPlayer = dynamic_cast<ComputerPlayer*>(player))
            {
                computerPlayer->setMoveSpeed(allComputer ? 60 : 20);
            }
        }
        else
        {
            throw std::invalid_argument("Invalid enum value for PlayerType.");
        }
    }
}


std::string Model::GetPlayerName(PlayerType)
{
    std::string result = mNames[mNamesIndex++ % mNames.size()];

    if (mNamesIndex >= mNames.size())
    {
        mNamesIndex = 0;
        std::shuffle(mNames.begin(), mNames.end(), mRandomEngine);
    }

    return result;
}


} // namespace Tetris
