#include "Model.h"
#include "Futile/Assert.h"
#include "Futile/AutoPtrSupport.h"
#include "Futile/Logging.h"
#include "Poco/Environment.h"


namespace QtTetris {


using namespace Futile;


Model::Model() :
    mQuit(false),
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
    Assert(mQuit);
}


void Model::quit()
{
    mMultiplayerGame.reset();
    mQuit = true;
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


std::size_t Model::playerCount() const
{
    return mMultiplayerGame->playerCount();
}


std::size_t Model::computerPlayerCount() const
{
    std::size_t result = 0;
    for (std::size_t idx = 0; idx < playerCount(); ++idx)
    {
        Player * player = mMultiplayerGame->getPlayer(idx);
        if (player->type() == PlayerType_Computer)
        {
            result++;
        }
    }
    return result;
}


Player * Model::getPlayer(std::size_t inIndex)
{
    return mMultiplayerGame->getPlayer(inIndex);
}


const MultiplayerGame & Model::multiplayerGame() const
{
    return *mMultiplayerGame;
}


MultiplayerGame & Model::multiplayerGame()
{
    if (!mMultiplayerGame)
    {
        throw std::runtime_error("There is currently no game running.");
    }
    return *mMultiplayerGame;
}


const Evaluator & Model::updateAIParameters(const Player & inPlayer,
                                            int & outSearchDepth,
                                            int & outSearchWidth,
                                            int & outWorkerCount,
                                            int & /*outMoveSpeed*/,
                                            BlockMover::MoveDownBehavior & outMoveDownBehavior)
{
    if (!inPlayer.simpleGame())
    {
        throw std::runtime_error("GameState is null!");
    }

    const Game & game = *inPlayer.simpleGame();

    if (mMultiplayerGame)
    {
        std::size_t numComputerPlayers = computerPlayerCount();
        if (numComputerPlayers == 0)
        {
            throw std::logic_error("Model::updateAIParameters: There are no computer players.");
        }

        outWorkerCount = Poco::Environment::processorCount() / numComputerPlayers;
        if (outWorkerCount == 0)
        {
            outWorkerCount = 1;
        }
    }
    else
    {
        outWorkerCount = 1;
    }

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
    if (currentHeight < 10)
    {
        outSearchDepth = 6;
        outSearchWidth = 6;
        return MakeTetrises::Instance();
    }
    else
    {
        outSearchDepth = 4;
        outSearchWidth = 4;
        return Survival::Instance();
    }
}


void Model::newGame(const PlayerTypes & inPlayerTypes, std::size_t inRowCount, std::size_t inColumnCount)
{
    mMultiplayerGame.reset();
    mMultiplayerGame.reset(new MultiplayerGame(inRowCount, inColumnCount));
    mGameOver = false;
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
        
        PlayerType playerType = inPlayerTypes[idx];
        if (playerType == PlayerType_Human)
        {
            mMultiplayerGame->addHumanPlayer(
                TeamName(teamName),
                PlayerName(GetPlayerName(inPlayerTypes[idx])));
        }
        else if (playerType == PlayerType_Computer)
        {
            Player * player = mMultiplayerGame->addComputerPlayer(
                TeamName(teamName),
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


} // namespace QtTetris