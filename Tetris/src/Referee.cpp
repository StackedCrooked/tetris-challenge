#include "Tetris/Referee.h"
#include "Tetris/MultiplayerGame.h"
#include <boost/noncopyable.hpp>


namespace Tetris {


struct Referee::Impl : boost::noncopyable
{
    Impl(MultiplayerGame & inMultiplayerGame) :
        mMultiplayerGame(inMultiplayerGame)
    {
    }

    MultiplayerGame & mMultiplayerGame;
};


Referee::Referee(MultiplayerGame & inMultiplayerGame) :
    mImpl(new Impl(inMultiplayerGame))
{
}


Referee::~Referee()
{
    delete mImpl;
}


} // namespace Tetris
