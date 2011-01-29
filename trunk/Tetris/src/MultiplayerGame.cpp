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
    boost::scoped_ptr<Referee> mReferee;
    std::set<ThreadSafe<Game> > mPlayers;
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


void MultiplayerGame::join(const ThreadSafe<Game> & inGame)
{
    mImpl->mPlayers.insert(inGame);
}


void MultiplayerGame::leave(const ThreadSafe<Game> & inGame)
{
    // calling erase(..) on a vector is slow,
    // but that should not be an issue here
    mImpl->mPlayers.erase(inGame);
}


} // namespace Tetris
