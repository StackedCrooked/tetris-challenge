#ifndef TETRIS_COMPUTERPLAYER_H
#define TETRIS_COMPUTERPLAYER_H


#include "Tetris/Player.h"
#include "Tetris/Computer.h"
#include <cstddef>


namespace Tetris {


class ComputerPlayer : public Player,
                       public Computer
{
public:
    static PlayerPtr Create(const TeamName & inTeamName,
                                   const PlayerName & inPlayerName,
                                   std::size_t inRowCount,
                                   std::size_t inColumnCount)
    {
        PlayerPtr result(new ComputerPlayer(inTeamName, inPlayerName, inRowCount, inColumnCount));
        return result;
    }

    ComputerPlayer(const TeamName & inTeamName,
                   const PlayerName & inPlayerName,
                   std::size_t inRowCount,
                   std::size_t inColumnCount);

    virtual ~ComputerPlayer() {}
};


} // namespace Tetris


#endif // TETRIS_COMPUTERPLAYER_H
