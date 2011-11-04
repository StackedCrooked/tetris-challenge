#ifndef WORKERPOOLTEST_H_INCLUDED
#define WORKERPOOLTEST_H_INCLUDED


#include "TetrisTest.h"


namespace testing {


class WorkerPoolTest : public TetrisTest
{
public:
    WorkerPoolTest();

protected:
    size_t mIterationCount;
};


} // namespace testing


#endif // WORKERPOOLTEST_H_INCLUDED
