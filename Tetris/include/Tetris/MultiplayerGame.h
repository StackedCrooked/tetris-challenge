#ifndef TETRIS_MULTIPLAYERGAME_H_INCLUDED
#define TETRIS_MULTIPLAYERGAME_H_INCLUDED


#include "Tetris/Player.h"
#include "Tetris/ComputerPlayer.h"
#include <boost/scoped_ptr.hpp>
#include <memory>
#include <set>


namespace Tetris {


class MultiplayerGame
{
public:
    MultiplayerGame(std::size_t inRowCount, std::size_t inColumnCount);

    ~MultiplayerGame();

    // Creates the player object and retains ownership
    Player * addHumanPlayer(const TeamName& inTeamName,
                            const PlayerName& inPlayerName);

    // Creates the player object and retains ownership
    Player * addComputerPlayer(const TeamName& inTeamName,
                               const PlayerName& inPlayerName,
                               ComputerPlayer::Tweaker* inTweaker);

    void removePlayer(Player* inPlayer);

    std::size_t playerCount() const;

    const Player * getPlayer(std::size_t inIndex) const;

    Player * getPlayer(std::size_t inIndex);

private:
    MultiplayerGame(const MultiplayerGame&);
    MultiplayerGame& operator=(const MultiplayerGame&);

    struct Impl;
    boost::scoped_ptr<Impl> mImpl;
};


} // namespace Tetris


#endif // TETRIS_MULTIPLAYERGAME_H_INCLUDED
