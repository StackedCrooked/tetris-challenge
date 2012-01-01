#ifndef TETRIS_PLAYER_H
#define TETRIS_PLAYER_H


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


FUTILE_BOX_TYPE(TeamName, std::string);
FUTILE_BOX_TYPE(PlayerName, std::string);


class Player;
typedef boost::shared_ptr<Player> PlayerPtr;


class Player : boost::noncopyable
{
public:
    static PlayerPtr Create(PlayerType inPlayerType,
                            const TeamName & inTeamName,
                            const PlayerName & inPlayerName,
                            std::size_t inRowCount,
                            std::size_t inColumnCount);

    virtual ~Player() = 0;

    inline PlayerType type() const { return mPlayerType; }

    inline const std::string & teamName() const { return mTeamName; }

    inline const std::string & playerName() const { return mPlayerName; }

    inline const SimpleGame & game() const { return mSimpleGame; }

    inline SimpleGame & game() { return mSimpleGame; }

protected:
    Player(PlayerType inPlayerType,
           const TeamName & inTeamName,
           const PlayerName & inPlayerName,
           std::size_t inRowCount,
           std::size_t inColumnCount) :
        mPlayerType(inPlayerType),
        mTeamName(inTeamName),
        mPlayerName(inPlayerName),
        mSimpleGame(inPlayerType, inRowCount, inColumnCount)
    {
    }

private:
    friend bool operator==(const Player & lhs, const Player & rhs);
    friend bool operator<(const Player & lhs, const Player & rhs);

    PlayerType mPlayerType;
    std::string mTeamName;
    std::string mPlayerName;
    SimpleGame mSimpleGame;
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


#endif // TETRIS_PLAYER_H
