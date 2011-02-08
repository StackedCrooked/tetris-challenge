#include "Tetris/Player.h"


namespace Tetris {


struct Player::Impl
{
    Impl(PlayerType inPlayerType,
         const TeamName & inTeamName,
         const PlayerName & inPlayerName,
         size_t inRowCount,
         size_t inColumnCount) :
        mRowCount(inRowCount),
        mColumnCount(inColumnCount),
        mPlayerType(inPlayerType),
        mSimpleGame(new SimpleGame(inRowCount, inColumnCount, inPlayerType)),
        mTeamName(inTeamName.get()),
        mPlayerName(inPlayerName.get())
    {
    }


    ~Impl()
    {
    }

    size_t mRowCount;
    size_t mColumnCount;
    PlayerType mPlayerType;
    boost::scoped_ptr<SimpleGame> mSimpleGame;
    std::string mTeamName;
    std::string mPlayerName;
};


Player::Player(PlayerType inPlayerType,
               const TeamName & inTeamName,
               const PlayerName & inPlayerName,
               size_t inRowCount,
               size_t inColumnCount) :
    mImpl(new Impl(inPlayerType,
                   inTeamName,
                   inPlayerName,
                   inRowCount,
                   inColumnCount))
{
}


Player::~Player()
{
    delete mImpl;
}


const std::string & Player::teamName() const
{
    return mImpl->mTeamName;
}


const std::string & Player::playerName() const
{
    return mImpl->mPlayerName;
}


const SimpleGame * Player::simpleGame() const
{
    return mImpl->mSimpleGame.get();
}


SimpleGame * Player::simpleGame()
{
    return mImpl->mSimpleGame.get();
}


void Player::resetGame()
{
    mImpl->mSimpleGame.reset();
    mImpl->mSimpleGame.reset(
        new SimpleGame(mImpl->mRowCount, mImpl->mColumnCount, mImpl->mPlayerType));
}


} // namespace Tetris
