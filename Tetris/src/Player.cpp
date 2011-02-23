#include "Tetris/Player.h"
#include "Tetris/ComputerPlayer.h"


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
        mSimpleGame(new SimpleGame(inPlayerType, inRowCount, inColumnCount)),
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


std::auto_ptr<Player> Player::Create(PlayerType inPlayerType,
                                     const TeamName & inTeamName,
                                     const PlayerName & inPlayerName,
                                     size_t inRowCount,
                                     size_t inColumnCount)
{
    std::auto_ptr<Player> result;
    if (inPlayerType == PlayerType_Computer)
    {
        result.reset(new ComputerPlayer(inTeamName, inPlayerName, inRowCount, inColumnCount));
    }
    else if (inPlayerType == PlayerType_Human)
    {
        result.reset(new HumanPlayer(inTeamName, inPlayerName, inRowCount, inColumnCount));
    }
    else
    {
        throw std::logic_error(MakeString() << "PlayerType: invalid enum value: " << inPlayerType);
    }
    return result;
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
}


Player::~Player()
{
    delete mImpl;
    mImpl = 0;
}


PlayerType Player::type() const
{
    return mImpl->mPlayerType;
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
    if (!mImpl->mSimpleGame)
    {
        return NULL;
    }
    return mImpl->mSimpleGame.get();
}


SimpleGame * Player::simpleGame()
{
    if (!mImpl->mSimpleGame)
    {
        return NULL;
    }
    return mImpl->mSimpleGame.get();
}


HumanPlayer::HumanPlayer(const TeamName & inTeamName,
                         const PlayerName & inPlayerName,
                         size_t inRowCount,
                         size_t inColumnCount) :
    Player(PlayerType_Human,
           inTeamName,
           inPlayerName,
           inRowCount,
           inColumnCount)
{
}


HumanPlayer::~HumanPlayer()
{
}


} // namespace Tetris
