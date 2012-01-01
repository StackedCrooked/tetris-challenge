#include "Tetris/Player.h"
#include "Tetris/Game.h"
#include "Tetris/ComputerPlayer.h"
#include "Futile/MakeString.h"


namespace Tetris {


using namespace Futile;


PlayerPtr Player::Create(PlayerType inPlayerType,
                         const TeamName & inTeamName,
                         const PlayerName & inPlayerName,
                         std::size_t inRowCount,
                         std::size_t inColumnCount)
{
    if (inPlayerType == PlayerType_Computer)
    {
        return ComputerPlayer::Create(inTeamName, inPlayerName, inRowCount, inColumnCount);
    }
    else if (inPlayerType == PlayerType_Human)
    {
        return HumanPlayer::Create(inTeamName, inPlayerName, inRowCount, inColumnCount);
    }
    throw std::logic_error(SS() << "PlayerType: invalid enum value: " << inPlayerType);
}


Player::~Player()
{
}


PlayerPtr HumanPlayer::Create(const TeamName & inTeamName,
                              const PlayerName & inPlayerName,
                              std::size_t inRowCount,
                              std::size_t inColumnCount)
{
    PlayerPtr result(new HumanPlayer(inTeamName, inPlayerName, inRowCount, inColumnCount));
    return result;
}


HumanPlayer::HumanPlayer(const TeamName & inTeamName,
                         const PlayerName & inPlayerName,
                         std::size_t inRowCount,
                         std::size_t inColumnCount) :
    Player(PlayerType_Human,
           inTeamName,
           inPlayerName,
           inRowCount,
           inColumnCount)
{
}


HumanPlayer::~HumanPlayer()
{
}


} // namespace Tetris
