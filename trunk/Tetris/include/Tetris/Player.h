#ifndef TETRIS_PLAYER_H_INCLUDED
#define TETRIS_PLAYER_H_INCLUDED


#include "Tetris/PlayerType.h"
#include "Tetris/Game.h"
#include "Futile/Array.h"
#include "Futile/TypedWrapper.h"
#include <boost/scoped_ptr.hpp>
#include <string>
#include <vector>


namespace Tetris {


Futile_TypedWrapper(TeamName, std::string);
Futile_TypedWrapper(PlayerName, std::string);


class Player
{
public:
    static std::auto_ptr<Player> Create(PlayerType inPlayerType,
                                        const TeamName & inTeamName,
                                        const PlayerName & inPlayerName,
                                        std::size_t inRowCount,
                                        std::size_t inColumnCount);

    Player(PlayerType inPlayerType,
           const TeamName & inTeamName,
           const PlayerName & inPlayerName,
           std::size_t inRowCount,
           std::size_t inColumnCount);

    virtual ~Player() = 0;

    PlayerType type() const;

    const std::string & teamName() const;

    const std::string & playerName() const;

    const Game * simpleGame() const;

    Game * simpleGame();

private:
    Player(const Player&);
    Player& operator=(const Player&);

    friend bool operator==(const Player & lhs, const Player & rhs);
    friend bool operator<(const Player & lhs, const Player & rhs);

    struct Impl;
    boost::scoped_ptr<Impl> mImpl;
};


class HumanPlayer : public Player
{
public:
    HumanPlayer(const TeamName & inTeamName,
                const PlayerName & inPlayerName,
                std::size_t inRowCount,
                std::size_t inColumnCount);

    virtual ~HumanPlayer();
};


} // namespace Tetris


#endif // TETRIS_PLAYER_H_INCLUDED
