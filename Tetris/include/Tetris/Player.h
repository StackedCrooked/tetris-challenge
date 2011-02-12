#ifndef TETRIS_PLAYER_H_INCLUDED
#define TETRIS_PLAYER_H_INCLUDED


#include "Tetris/Array.h"
#include "Tetris/PlayerType.h"
#include "Tetris/SimpleGame.h"
#include "Tetris/TypedWrapper.h"
#include <string>
#include <vector>


namespace Tetris {


Tetris_TypedWrapper(TeamName, std::string);
Tetris_TypedWrapper(PlayerName, std::string);


class Player
{
public:
    Player(PlayerType inPlayerType,
           const TeamName & inTeamName,
           const PlayerName & inPlayerName,
           size_t inRowCount,
           size_t inColumnCount);

    ~Player();

    const std::string & teamName() const;

    const std::string & playerName() const;

    const SimpleGame * simpleGame() const;

    SimpleGame * simpleGame();

    void resetGame();

private:
    Player(const Player&);
    Player& operator=(const Player&);

    friend bool operator==(const Player & lhs, const Player & rhs);
    friend bool operator<(const Player & lhs, const Player & rhs);

    struct Impl;
    Impl * mImpl;
};


typedef Array<Player> Players;


} // namespace Tetris


#endif // TETRIS_PLAYER_H_INCLUDED
