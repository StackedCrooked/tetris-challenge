#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED


#include "Game.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <vector>


namespace Tetris
{

    typedef boost::function<void(void*)> Job;

    class JobList
    {
    public:
        const Job & get(size_t inIndex) const;

        Job & get(size_t inIndex);

        void add(const Job & inJob);

        void clear();

        size_t size() const;

        bool empty() const;

    private:
        std::vector<Job> mJobs;
        mutable boost::mutex mMutex;
    };

    class Player
    {
    public:
        Player(Game * inGame);

        void setMaxThreadCount(size_t inMaxThreadCount);

        void doJobsAndWait(const JobList & inJobs);

        void move(const std::vector<int> & inWidths);

        void playUntilGameOver(const std::vector<int> & inDepths);

        void populateNodeMultiThreaded(GameStateNode & inNode, const BlockTypes & inBlocks, const std::vector<int> & inWidths, size_t inOffset);

    private:
        Game * mGame;
        size_t mMaxThreadCount;
        boost::thread_group mThreadGroup;
        boost::mutex mMutex;
    };

} // namespace Tetris

#endif // PLAYER_H_INCLUDED
