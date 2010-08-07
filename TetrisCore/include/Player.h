#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED


#include "Game.h"
#include "JobList.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <vector>


namespace Tetris
{
    
    typedef std::vector<int> Widths;

    class Player
    {
    public:
        Player(Game * inGame);

        void move(const std::vector<int> & inWidths);

        void playUntilGameOver(const std::vector<int> & inWidths);

    private:
        Game * mGame;
        boost::thread_group mThreadGroup;
        boost::mutex mMutex;
    };

} // namespace Tetris

#endif // PLAYER_H_INCLUDED
