#ifndef TETRIS_MULTIPLAYERGAME_H_INCLUDED
#define TETRIS_MULTIPLAYERGAME_H_INCLUDED


#include "Tetris/SimpleGame.h"
#include <set>


namespace Tetris {


class SimpleGame;
template<class T> class ThreadSafe;


class MultiplayerGame
{
public:
    MultiplayerGame();

    ~MultiplayerGame();

    typedef std::set<SimpleGame*> Games;

    void join(SimpleGame & inGame);

    void leave(SimpleGame & inGame);

    const Games & games() const;

private:
    struct Impl;
    Impl * mImpl;
};


} // namespace Tetris


#endif // TETRIS_MULTIPLAYERGAME_H_INCLUDED
