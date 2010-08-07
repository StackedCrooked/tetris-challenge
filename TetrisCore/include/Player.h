#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED


#include "Game.h"
#include "JobList.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <ostream>
#include <vector>


namespace Tetris
{
    
    typedef std::vector<int> Widths;

    class Player
    {
    public:
        Player(Game * inGame);

        void move(const std::vector<int> & inWidths);

        void setLogger(std::ostream & inOutStream);

        void playUntilGameOver(const std::vector<int> & inWidths);

    private:
        void log(const std::string & inMessage);

        Game * mGame;
        boost::thread_group mThreadGroup;
        boost::mutex mMutex;
        std::ostream * mOutStream;
    };

} // namespace Tetris

#endif // PLAYER_H_INCLUDED
