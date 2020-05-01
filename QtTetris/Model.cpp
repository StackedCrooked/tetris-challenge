#include "Model.h"
#include "Futile/AutoPtrSupport.h"
#include "Poco/Environment.h"


namespace Tetris {


Model::Model() :
    mNames(),
    mNamesIndex(0),
    mHumanName(),
    mCPUCount(Poco::Environment::processorCount()),
    mGameOver(true)
{
    mNames.push_back("Luffy");
    mNames.push_back("Zoro");
    mNames.push_back("Nami");
    mNames.push_back("Sanji");
    mNames.push_back("Chopper");
    srand(static_cast<unsigned int>(time(0)));
    std::random_shuffle(mNames.begin(), mNames.end());
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
        Player * player = mMultiplayerGame->getPlayer(idx);
        if (player->simpleGame()->isGameOver())
        {
            mGameOver = true;
        }
    }
    return mGameOver;
}


Player * Model::getPlayer(std::size_t inIndex)
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

    static const auto num_cpus = Poco::Environment::processorCount();

    outWorkerCount = std::max(1, static_cast<int>((num_cpus - 2) / 2));

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

    // Tactics adjustment
    if (currentHeight < 8)
    {
        outSearchDepth = 20;
        outSearchWidth = 2;
        outMoveSpeed = 1000;
        return MakeTetrises::Instance();
    }
    else
    {
        outSearchDepth = 4;
        outSearchWidth = 4;
        outMoveSpeed = 1000;
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

        Player * player(0);
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
            if (ComputerPlayer * computerPlayer = dynamic_cast<ComputerPlayer*>(player))
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


std::string Model::GetHumanPlayerName()
{
    return "Luffy";
}


std::string Model::GetPlayerName(PlayerType inPlayerType)
{
    if (inPlayerType == Human)
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


} // namespace Tetris

