#include "Tetris/Referee.h"
#include "Tetris/Game.h"
#include "Tetris/MultiplayerGame.h"
#include "Tetris/Threading.h"
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>


namespace Tetris {


struct Referee::Impl : boost::noncopyable
{
    typedef MultiplayerGame::Players Players;
    typedef MultiplayerGame::Player Player;

    Impl(MultiplayerGame & inMultiplayerGame) :
        mMultiplayerGame(inMultiplayerGame)
    {
    }

    void onPlayerJoined(const ThreadSafe<Game> & inPlayer)
    {
        // Make a (shallow) copy to make the const go away.
        ThreadSafe<Game> copy(inPlayer);
        ScopedReaderAndWriter<Game> rwgame(copy);
        Game & game(*rwgame.get());
        game.OnLinesCleared.connect(
            boost::bind(&Referee::Impl::onLinesCleared, this, inPlayer, _1));
    }

    void onPlayerLeft(const ThreadSafe<Game> & )
    {
    }

    void onLinesCleared(ThreadSafe<Game> & inPlayer, int inLines);

    MultiplayerGame & mMultiplayerGame;
};


Referee::Referee(MultiplayerGame & inMultiplayerGame) :
    mImpl(new Impl(inMultiplayerGame))
{
    mImpl->mMultiplayerGame.OnPlayerJoined.connect(
        boost::bind(&Referee::Impl::onPlayerJoined, mImpl, _1));
}


Referee::~Referee()
{
    delete mImpl;
}


void Referee::Impl::onLinesCleared(ThreadSafe<Game> & inPlayer, int inLines)
{
    // If number of lines >= 2 then apply a line penalty to each non-allied player.
    // Note: we currently assume that all other players are non-allied.

    // Take a copy, it's ok.
    Players players = mMultiplayerGame.players();
    Players::iterator it = players.begin(), end = players.end();
    for (; it != end; ++it)
    {
        Player player(*it);
        if (player != inPlayer)
        {
            ScopedReaderAndWriter<Game> rwgame(player);
            Game & game(*rwgame.get());
            game.applyLinePenalty(inLines);
        }
    }
}


} // namespace Tetris
