#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED


#include "Game.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <vector>


namespace Tetris
{

    typedef boost::function<void(void*)> Job;
    typedef std::vector<Job> Jobs;

    class Player
    {
    public:
        Player(Game * inGame);

        void setMaxThreadCount(size_t inMaxThreadCount);

        void move(const std::vector<int> & inWidths);

        void playUntilGameOver(const std::vector<int> & inDepths);

        void populateNodeMultiThreaded(GameStateNode & inNode, const BlockTypes & inBlocks, const std::vector<int> & inWidths, size_t inOffset);

    private:
        Game * mGame;
        size_t mMaxThreadCount;
        Jobs mJobs;
        boost::mutex mMutex;
    };

} // namespace Tetris

#endif // PLAYER_H_INCLUDED
