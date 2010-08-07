#ifndef JOBLIST_H_INCLUDED
#define JOBLIST_H_INCLUDED


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


} // namespace Tetris

#endif // JOBLIST_H_INCLUDED
