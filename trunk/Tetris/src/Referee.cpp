//#include "Tetris/Referee.h"
//#include "Tetris/Game.h"
//#include "Tetris/MultiplayerGame.h"
//#include "Tetris/Threading.h"
//#include <boost/bind.hpp>
//#include <boost/noncopyable.hpp>


//namespace Tetris {


//struct Referee::Impl : boost::noncopyable,
//                       public Game::EventHandler

//{
//    typedef MultiplayerGame::Players Players;
//    typedef MultiplayerGame::Player Player;

//    Impl(MultiplayerGame & inMultiplayerGame) :
//        mMultiplayerGame(inMultiplayerGame)
//    {
//        const MultiplayerGame::Players & players = inMultiplayerGame.players();
//        Players::const_iterator it = players.begin(), end = players.end();
//        for (; it != end; ++it)
//        {
//            const MultiplayerGame::Player & player(*it);
//            const ThreadSafe<Game> & game(player);
//            ScopedReader<Game> rgame(game);
//            const Game * pGame(rgame.get());
//            pGame->registerEventHandler(this);
//        }
//    }

//    ~Impl()
//    {
//    }

//    MultiplayerGame & mMultiplayerGame;
//};


//Referee::Referee(MultiplayerGame & inMultiplayerGame) :
//    mImpl(new Impl(inMultiplayerGame))
//{
//}


//Referee::~Referee()
//{
//    delete mImpl;
//}


//} // namespace Tetris
