#include "Tetris/Config.h"
#include "Tetris/Player.h"
#include "Tetris/ComputerPlayer.h"
#include "Futile/MakeString.h"


using Futile::MakeString;


namespace Tetris {


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
        mGame(new Game(inPlayerType, inRowCount, inColumnCount)),
        mTeamName(inTeamName.get()),
        mPlayerName(inPlayerName.get())
    {
    }


    ~Impl()
    {
    }

    std::size_t mRowCount;
    std::size_t mColumnCount;
    PlayerType mPlayerType;
    boost::scoped_ptr<Game> mGame;
    std::string mTeamName;
    std::string mPlayerName;
};


std::unique_ptr<Player> Player::Create(PlayerType inPlayerType,
                                     const TeamName & inTeamName,
                                     const PlayerName & inPlayerName,
                                     std::size_t inRowCount,
                                     std::size_t inColumnCount)
{
    std::unique_ptr<Player> result;
    if (inPlayerType == Computer)
    {
        result.reset(new ComputerPlayer(inTeamName, inPlayerName, inRowCount, inColumnCount));
    }
    else if (inPlayerType == Human)
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


const Game * Player::simpleGame() const
{
    return mImpl->mGame.get();
}


Game * Player::simpleGame()
{
    return mImpl->mGame.get();
}


HumanPlayer::HumanPlayer(const TeamName & inTeamName,
                         const PlayerName & inPlayerName,
                         std::size_t inRowCount,
                         std::size_t inColumnCount) :
    Player(Human,
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
