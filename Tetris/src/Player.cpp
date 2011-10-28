#include "Tetris/Player.h"
#include "Tetris/ComputerPlayer.h"
#include "Futile/MakeString.h"


namespace Tetris {


using namespace Futile;


struct Player::Impl
{
    Impl(PlayerType inPlayerType,
         const TeamName & inTeamName,
         const PlayerName & inPlayerName,
         std::size_t inRowCount,
         std::size_t inColumnCount) :
        mRowCount(inRowCount),
        mColumnCount(inColumnCount),
        mPlayerType(inPlayerType),
        mGame(new SimpleGame(inPlayerType, inRowCount, inColumnCount)),
        mTeamName(inTeamName),
        mPlayerName(inPlayerName)
    {
    }


    ~Impl()
    {
    }

    std::size_t mRowCount;
    std::size_t mColumnCount;
    PlayerType mPlayerType;
    boost::scoped_ptr<SimpleGame> mGame;
    std::string mTeamName;
    std::string mPlayerName;
};


PlayerPtr Player::Create(PlayerType inPlayerType,
                         const TeamName & inTeamName,
                         const PlayerName & inPlayerName,
                         std::size_t inRowCount,
                         std::size_t inColumnCount)
{
    if (inPlayerType == PlayerType_Computer)
    {
        return ComputerPlayer::Create(inTeamName, inPlayerName, inRowCount, inColumnCount);
    }
    else if (inPlayerType == PlayerType_Human)
    {
        return HumanPlayer::Create(inTeamName, inPlayerName, inRowCount, inColumnCount);
    }
    throw std::logic_error(SS() << "PlayerType: invalid enum value: " << inPlayerType);
}


Player::Player(PlayerType inPlayerType,
               const TeamName & inTeamName,
               const PlayerName & inPlayerName,
               std::size_t inRowCount,
               std::size_t inColumnCount) :
    mImpl(new Impl(inPlayerType,
                   inTeamName,
                   inPlayerName,
                   inRowCount,
                   inColumnCount))
{
}


Player::~Player()
{
    mImpl.reset();
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


const SimpleGame * Player::game() const
{
    return mImpl->mGame.get();
}


SimpleGame * Player::game()
{
    return mImpl->mGame.get();
}


PlayerPtr HumanPlayer::Create(const TeamName & inTeamName,
                              const PlayerName & inPlayerName,
                              std::size_t inRowCount,
                              std::size_t inColumnCount)
{
    PlayerPtr result(new HumanPlayer(inTeamName, inPlayerName, inRowCount, inColumnCount));
    return result;
}


HumanPlayer::HumanPlayer(const TeamName & inTeamName,
                         const PlayerName & inPlayerName,
                         std::size_t inRowCount,
                         std::size_t inColumnCount) :
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
