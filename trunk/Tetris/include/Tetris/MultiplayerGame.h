#ifndef TETRIS_MULTIPLAYERGAME_H_INCLUDED
#define TETRIS_MULTIPLAYERGAME_H_INCLUDED


#include "Tetris/Player.h"
#include <set>


namespace Tetris {


class MultiplayerGame
{
public:
    MultiplayerGame();

    ~MultiplayerGame();

    typedef std::set<Player> Players;

    void join(Player inPlayer);

    void leave(Player inPlayer);

    const Players & players() const;

private:
    MultiplayerGame(const MultiplayerGame&);
    MultiplayerGame& operator=(const MultiplayerGame&);

    struct Impl;
    Impl * mImpl;
};


} // namespace Tetris


#endif // TETRIS_MULTIPLAYERGAME_H_INCLUDED
