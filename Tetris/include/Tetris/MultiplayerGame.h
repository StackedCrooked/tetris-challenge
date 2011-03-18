#ifndef TETRIS_MULTIPLAYERGAME_H_INCLUDED
#define TETRIS_MULTIPLAYERGAME_H_INCLUDED


#include "Tetris/Player.h"
#include <boost/scoped_ptr.hpp>
#include <memory>
#include <set>


namespace Tetris {


class MultiplayerGame
{
public:
    MultiplayerGame(size_t inRowCount, size_t inColumnCount);

    ~MultiplayerGame();

    // Takes ownership!
    Player * addPlayer(PlayerType inPlayerType,
                       const TeamName & inTeamName,
                       const PlayerName & inPlayerName);

    void removePlayer(Player * inPlayer);

    size_t playerCount() const;

    const Player * getPlayer(size_t inIndex) const;

    Player * getPlayer(size_t inIndex);

private:
    MultiplayerGame(const MultiplayerGame&);
    MultiplayerGame& operator=(const MultiplayerGame&);

    struct Impl;
    boost::scoped_ptr<Impl> mImpl;
};


} // namespace Tetris


#endif // TETRIS_MULTIPLAYERGAME_H_INCLUDED
