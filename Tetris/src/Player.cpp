#include "Tetris/Player.h"


namespace Tetris {


struct Player::Impl
{
    Impl(PlayerType inPlayerType,
         const TeamName & inTeamName,
         const PlayerName & inPlayerName,
         size_t inRowCount,
         size_t inColumnCount) :
        mSimpleGame(inRowCount, inColumnCount, inPlayerType),
        mTeamName(inTeamName.get()),
        mPlayerName(inPlayerName.get()),
        mRefCount(1)
    {
    }


    ~Impl()
    {
    }

    SimpleGame mSimpleGame;
    std::string mTeamName;
    std::string mPlayerName;
    int mRefCount;
};


bool operator==(const Player & lhs, const Player & rhs)
{
    return lhs.mImpl == rhs.mImpl;
}


bool operator<(const Player & lhs, const Player & rhs)
{
    return lhs.mImpl < rhs.mImpl;
}


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
    Assert(mImpl->mRefCount >= 1);
}


Player::~Player()
{
    Assert(mImpl->mRefCount >= 1);
    if (0 == --mImpl->mRefCount)
    {
        delete mImpl;
    }
    Assert(mImpl->mRefCount >= 1);
}


Player::Player(const Player & rhs) :
    mImpl(rhs.mImpl)
{
    mImpl->mRefCount++;

    Assert(mImpl == rhs.mImpl);
    Assert(mImpl->mRefCount == rhs.mImpl->mRefCount);
}


Player & Player::operator=(const Player & rhs)
{
    Assert(mImpl->mRefCount >= 1);
    if (this != &rhs)
    {
        if (0 == --mImpl->mRefCount)
        {
            delete mImpl;
        }
        rhs.mImpl->mRefCount++;
        mImpl = rhs.mImpl;
    }
    Assert(mImpl->mRefCount >= 1);
    Assert(mImpl == rhs.mImpl);
    Assert(mImpl->mRefCount == rhs.mImpl->mRefCount);
    return *this;
}


const std::string & Player::teamName() const
{
    return mImpl->mTeamName;
}


const std::string & Player::playerName() const
{
    return mImpl->mPlayerName;
}


const SimpleGame & Player::simpleGame() const
{
    return mImpl->mSimpleGame;
}


SimpleGame & Player::simpleGame()
{
    return mImpl->mSimpleGame;
}


} // namespace Tetris
