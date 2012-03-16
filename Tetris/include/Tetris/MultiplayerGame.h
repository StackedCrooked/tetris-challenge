#ifndef TETRIS_MULTIPLAYERGAME_H
#define TETRIS_MULTIPLAYERGAME_H


#include "Tetris/Player.h"
#include "Tetris/ComputerPlayer.h"
#include <boost/scoped_ptr.hpp>


namespace Tetris {


class MultiplayerGame
{
public:
    MultiplayerGame(unsigned inRowCount, unsigned inColumnCount);

    ~MultiplayerGame();

    // Creates the player object and retains ownership
    Player & addHumanPlayer(const TeamName & inTeamName,
                            const PlayerName & inPlayerName);

    // Creates the player object and retains ownership
    Player & addComputerPlayer(const TeamName & inTeamName,
                               const PlayerName & inPlayerName);

    void removePlayer(Player * inPlayer);

    unsigned playerCount() const;

    unsigned playerCount(PlayerType inPlayerType) const;

    const Player * getPlayer(unsigned inIndex) const;

    Player * getPlayer(unsigned inIndex);

    unsigned rowCount() const;

    unsigned columnCount() const;

private:
    MultiplayerGame(const MultiplayerGame&);
    MultiplayerGame& operator=(const MultiplayerGame&);

    struct Impl;
    boost::scoped_ptr<Impl> mImpl;
};


} // namespace Tetris


#endif // TETRIS_MULTIPLAYERGAME_H
