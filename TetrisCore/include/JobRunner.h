#ifndef JOBRUNNER_H_INCLUDED
#define JOBRUNNER_H_INCLUDED


#include "JobList.h"
#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>
#include <memory>


namespace Tetris
{

    /**
     * JobRunner runs a jobs sequentially in separate thread.
     *
     * The JobRunner object is is idle until a job is pushed using the push()
     * method. If multiple jobs are pushed then the JobRunner will process
     * them sequentially. Once all jobs have been processed the JobRunner
     * remains idle until the next push.
     */
    class JobRunner
    {
    public:
        JobRunner();

        // Use this method to start the runner. You only need to do this once.
        void start();

        void stop();

        void push(const Job & inJob);

        bool isIdle() const;

    private:
        bool isStopped() const;

        boost::scoped_ptr<JobList> mJobList;
        boost::scoped_ptr<boost::thread> mThread;
        boost::mutex mRunnerMutex;
        boost::condition_variable mJobsAvailableCondition;
        bool mIsStopped;
        mutable boost::mutex mIsStoppedMutex;
    };


} // namespace Tetris

#endif // JOBRUNNER_H_INCLUDED
