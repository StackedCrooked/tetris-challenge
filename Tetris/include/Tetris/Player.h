#ifndef TETRIS_PLAYER_H_INCLUDED
#define TETRIS_PLAYER_H_INCLUDED


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
           size_t inRowCount = 20,
           size_t inColumnCount = 10);

    ~Player();

    Player(const Player & rhs);

    Player & operator=(const Player & rhs);

    const std::string & teamName() const;

    const std::string & playerName() const;

    const SimpleGame & simpleGame() const;

    SimpleGame & simpleGame();

private:
    friend bool operator==(const Player & lhs, const Player & rhs);
    friend bool operator<(const Player & lhs, const Player & rhs);

    struct Impl;
    Impl * mImpl; // ref-counted
};

typedef std::vector<Player> Players;

bool operator==(const Player & lhs, const Player & rhs);

bool operator<(const Player & lhs, const Player & rhs);


} // namespace Tetris


#endif // TETRIS_PLAYER_H_INCLUDED
