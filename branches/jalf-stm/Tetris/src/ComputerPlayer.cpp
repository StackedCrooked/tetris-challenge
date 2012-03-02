#include "Tetris/ComputerPlayer.h"


namespace Tetris {


using namespace Futile;


ComputerPlayer::ComputerPlayer(const TeamName & inTeamName,
                               const PlayerName & inPlayerName,
                               std::size_t inRowCount,
                               std::size_t inColumnCount) :
    Player(PlayerType_Computer, inTeamName, inPlayerName, inRowCount, inColumnCount),
    Computer(Player::game().game())
{
}


} // namespace ComputerPlayer
