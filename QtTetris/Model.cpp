#include "Model.h"
#include "Futile/Assert.h"
#include "Futile/AutoPtrSupport.h"
#include "Futile/Logging.h"
#include "Poco/Environment.h"
#include <algorithm>
#include <string>
#include <vector>


namespace QtTetris {


using namespace Futile;


Model::Model() :
    mNames(),
    mNamesIndex(0),
    mHumanName(),
    mCPUCount(Poco::Environment::processorCount()),
    mGameOver(true)
{
    mNames.push_back("Chopper");
    mNames.push_back("Dragon");
    mNames.push_back("Luffy");
    mNames.push_back("Nami");
    mNames.push_back("Robin");
    mNames.push_back("Sanji");
    mNames.push_back("Shanks");
    mNames.push_back("Usopp");
    mNames.push_back("Zoro");

    srand(static_cast<unsigned int>(time(0)));
    std::random_shuffle(mNames.begin(), mNames.end());
}


Model::~Model()
{
    mMultiplayerGame.reset();
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
        if (player->game().isGameOver())
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


namespace {


#if POCO_OS_MAC_OS_X
#define FORCE_TETRIS_WORKER_COUNT 1
#else
#define FORCE_TETRIS_WORKER_COUNT 0
#endif


//unsigned CalculateOptimalWorkerCount(std::size_t numComputerPlayers)
//{
//#if FORCE_TETRIS_WORKER_COUNT != 0
//    (void)numComputerPlayers;
//    return FORCE_TETRIS_WORKER_COUNT;
//#else
//    if (numComputerPlayers == 0)
//    {
//        throw std::invalid_argument("numComputerPlayers == 0");
//    }

//    return std::max<unsigned>(Poco::Environment::processorCount() / numComputerPlayers, 2) - 1;
//#endif
//}


} // anonymous namespace


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
        if (idx != 0 && idx >= inPlayerTypes.size() / 2)
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
            mMultiplayerGame->addComputerPlayer(TeamName(teamName), PlayerName(GetPlayerName(inPlayerTypes[idx])));
        }
        else
        {
            throw std::invalid_argument("Invalid enum value for PlayerType.");
        }
    }

    //const unsigned cComputerPlayerCount = mMultiplayerGame->playerCount(PlayerType_Computer);
    for (PlayerPtr player : mMultiplayerGame->getPlayers())
    {
        if (ComputerPlayer * computerPlayer = dynamic_cast<ComputerPlayer*>(player.get()))
        {
            computerPlayer->setMoveSpeed(allComputer ? 100 : 10);
            computerPlayer->setWorkerCount(7);//CalculateOptimalWorkerCount(cComputerPlayerCount));
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
            GetHumanPlayerName().swap(mHumanName);
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
