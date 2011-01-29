#ifndef TETRIS_MULTIPLAYERGAME_H_INCLUDED
#define TETRIS_MULTIPLAYERGAME_H_INCLUDED


#include <boost/signals2/signal.hpp>
#include <set>


namespace Tetris {


class Game;
template<class T> class ThreadSafe;


class MultiplayerGame
{
public:
    boost::signals2::signal<void(const ThreadSafe<Game> &)> OnPlayerJoined;

    boost::signals2::signal<void(const ThreadSafe<Game> &)> OnPlayerLeft;

    MultiplayerGame();

    ~MultiplayerGame();

    typedef ThreadSafe<Game> Player;

    typedef std::set<Player> Players;

    void join(const Player & inPlayer);

    void leave(const Player & inPlayer);

    const Players & players() const;

private:
    struct Impl;
    Impl * mImpl;
};


} // namespace Tetris


#endif // TETRIS_MULTIPLAYERGAME_H_INCLUDED
