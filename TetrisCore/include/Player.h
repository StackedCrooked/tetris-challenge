#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED


#include "Game.h"
#include "ThreadSafeGame.h"
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
        Player(const ThreadSafeGame & inThreadSafeGame);

        void move(const std::vector<int> & inWidths);

        void setLogger(std::ostream & inOutStream);

        void playUntilGameOver(const std::vector<int> & inWidths);

    private:
        void log(const std::string & inMessage);

        ThreadSafeGame mThreadSafeGame;
        std::ostream * mOutStream;
    };

} // namespace Tetris

#endif // PLAYER_H_INCLUDED
