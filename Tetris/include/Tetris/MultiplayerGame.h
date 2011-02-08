#ifndef TETRIS_MULTIPLAYERGAME_H_INCLUDED
#define TETRIS_MULTIPLAYERGAME_H_INCLUDED


#include "Tetris/Player.h"
#include <memory>
#include <set>


namespace Tetris {


class MultiplayerGame
{
public:
    MultiplayerGame();

    ~MultiplayerGame();

    // Takes ownership!
    Player * join(std::auto_ptr<Player> inPlayer);

    void leave(Player * inPlayer);

    size_t playerCount() const;

    const Player * getPlayer(size_t inIndex) const;

    Player * getPlayer(size_t inIndex);

private:
    MultiplayerGame(const MultiplayerGame&);
    MultiplayerGame& operator=(const MultiplayerGame&);

    struct Impl;
    Impl * mImpl;
};


} // namespace Tetris


#endif // TETRIS_MULTIPLAYERGAME_H_INCLUDED
