#ifndef JOBLIST_H_INCLUDED
#define JOBLIST_H_INCLUDED


#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include <queue>


namespace Tetris
{


    /**
     * JobList
     *
     * Executes jobs (functions) sequentially in a separate thread.
     */
    class JobList
    {
    public:        
        typedef boost::function<void()> Job;

        typedef boost::function<void(int)> Callback;

        JobList();

        // Returns jobid, which will be passed to the callback once the job has finished running.
        //int push(std::auto_ptr<Job> inJob, Callback inCallback);

        size_t size() const;

        bool empty() const;

    private:
        struct Task
        {
            int id;
            Job job;
        };
        void start();

        std::queue<boost::shared_ptr<Job> > mJobs;
        boost::scoped_ptr<boost::thread> mThread;
        mutable boost::condition_variable mJobsInQueue;
        mutable boost::mutex mMutex;
    };


} // namespace Tetris

#endif // JOBLIST_H_INCLUDED
