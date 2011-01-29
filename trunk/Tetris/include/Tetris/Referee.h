#ifndef TETRIS_REFEREE_H_INCLUDED
#define TETRIS_REFEREE_H_INCLUDED


namespace Tetris {


class MultiplayerGame;


/**
 * Referee watches over a multiplayer game and
 * passes bonuses or penalties to the players.
 */
class Referee
{
public:
    Referee(MultiplayerGame & inMultiplayerGame);

    ~Referee();

private:
    Referee(const Referee&);
    Referee& operator=(const Referee&);

    struct Impl;
    Impl * mImpl;
};


} // namespace Tetris


#endif // TETRIS_REFEREE_H_INCLUDED
