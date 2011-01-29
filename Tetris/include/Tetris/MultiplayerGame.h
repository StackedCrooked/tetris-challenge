#ifndef TETRIS_MULTIPLAYERGAME_H_INCLUDED
#define TETRIS_MULTIPLAYERGAME_H_INCLUDED


namespace Tetris {


class Game;
template<class T> class ThreadSafe;


class MultiplayerGame
{
public:
    MultiplayerGame();

    ~MultiplayerGame();

    void join(const ThreadSafe<Game> & inGame);

    void leave(const ThreadSafe<Game> & inGame);

private:
    struct Impl;
    Impl * mImpl;
};


} // namespace Tetris


#endif // TETRIS_MULTIPLAYERGAME_H_INCLUDED
