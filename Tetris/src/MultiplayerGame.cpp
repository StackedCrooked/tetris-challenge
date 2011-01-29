#include "Tetris/MultiplayerGame.h"
#include "Tetris/Referee.h"
#include "Tetris/Threading.h"
#include "Tetris/Game.h"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <algorithm>
#include <vector>
#include <set>


namespace Tetris {


struct MultiplayerGame::Impl : boost::noncopyable
{
    typedef MultiplayerGame::Players Players;

    boost::scoped_ptr<Referee> mReferee;
    Players mPlayers;
};


MultiplayerGame::MultiplayerGame() :
    mImpl(new Impl)
{
    mImpl->mReferee.reset(new Referee(*this));
}


MultiplayerGame::~MultiplayerGame()
{
    delete mImpl;
}


void MultiplayerGame::join(const Player & inPlayer)
{
    mImpl->mPlayers.insert(inPlayer);
    OnPlayerJoined(inPlayer);
}


void MultiplayerGame::leave(const Player & inPlayer)
{
    // calling erase(..) on a vector is slow,
    // but that should not be an issue here
    mImpl->mPlayers.erase(inPlayer);
    OnPlayerLeft(inPlayer);
}


const MultiplayerGame::Players & MultiplayerGame::players() const
{
    return mImpl->mPlayers;
}


} // namespace Tetris
