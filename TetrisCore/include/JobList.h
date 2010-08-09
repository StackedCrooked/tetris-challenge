#ifndef JOBLIST_H_INCLUDED
#define JOBLIST_H_INCLUDED


#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <queue>


namespace Tetris
{

    typedef boost::function<void()> Job;

    class JobList
    {
    public:
        const Job & peek() const;

        Job pop();

        void push(const Job & inJob);

        size_t size() const;

        bool empty() const;

    private:
        std::queue<Job> mJobs;
        mutable boost::mutex mMutex;
    };


} // namespace Tetris

#endif // JOBLIST_H_INCLUDED
