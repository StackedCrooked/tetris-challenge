#ifndef TETRIS_PLAYER_H_INCLUDED
#define TETRIS_PLAYER_H_INCLUDED


#include "Tetris/PlayerType.h"
#include "Tetris/SimpleGame.h"
#include "Futile/Array.h"
#include "Futile/TypedWrapper.h"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <string>
#include <vector>


namespace Tetris {


Futile_TypedWrapper(TeamName, std::string);
Futile_TypedWrapper(PlayerName, std::string);


class Player;
typedef boost::shared_ptr<Player> PlayerPtr;


class Player : public boost::enable_shared_from_this<Player>,
               boost::noncopyable
{
public:
    static PlayerPtr Create(PlayerType inPlayerType,
                            const TeamName & inTeamName,
                            const PlayerName & inPlayerName,
                            std::size_t inRowCount,
                            std::size_t inColumnCount);

    virtual ~Player() = 0;

    PlayerType type() const;

    const std::string & teamName() const;

    const std::string & playerName() const;

    const SimpleGame * game() const;

    SimpleGame * game();

protected:
    Player(PlayerType inPlayerType,
           const TeamName & inTeamName,
           const PlayerName & inPlayerName,
           std::size_t inRowCount,
           std::size_t inColumnCount);

private:
    friend bool operator==(const Player & lhs, const Player & rhs);
    friend bool operator<(const Player & lhs, const Player & rhs);

    struct Impl;
    boost::scoped_ptr<Impl> mImpl;
};


class HumanPlayer : public Player
{
public:
    static PlayerPtr Create(const TeamName & inTeamName,
                            const PlayerName & inPlayerName,
                            std::size_t inRowCount,
                            std::size_t inColumnCount);

    virtual ~HumanPlayer();

private:
    HumanPlayer(const TeamName & inTeamName,
                const PlayerName & inPlayerName,
                std::size_t inRowCount,
                std::size_t inColumnCount);
};


} // namespace Tetris


#endif // TETRIS_PLAYER_H_INCLUDED
