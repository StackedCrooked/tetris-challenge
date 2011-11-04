#ifndef WORKERPOOLTEST_H_INCLUDED
#define WORKERPOOLTEST_H_INCLUDED


#include "TetrisTest.h"
#include "Poco/Stopwatch.h"
#include "Poco/Types.h"


class WorkerPoolTest : public TetrisTest
{
public:
    WorkerPoolTest();

protected:
    size_t mIterationCount;
};


#endif // WORKERPOOLTEST_H_INCLUDED
