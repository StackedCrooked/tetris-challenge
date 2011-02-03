#ifndef TETRIS_MULTIPLAYERGAME_H_INCLUDED
#define TETRIS_MULTIPLAYERGAME_H_INCLUDED


#include <set>


namespace Tetris {


class Game;
template<class T> class ThreadSafe;


class MultiplayerGame
{
public:
    MultiplayerGame();

    ~MultiplayerGame();

    typedef std::set< ThreadSafe<Game> > Games;

    void join(ThreadSafe<Game> inGame);

    void leave(ThreadSafe<Game> inGame);

    const Games & games() const;

private:
    struct Impl;
    Impl * mImpl;
};


} // namespace Tetris


#endif // TETRIS_MULTIPLAYERGAME_H_INCLUDED
