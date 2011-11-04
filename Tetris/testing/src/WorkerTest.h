#ifndef WORKERTEST_H_INCLUDED
#define WORKERTEST_H_INCLUDED


#include "TetrisTest.h"


namespace testing {


class WorkerTest : public TetrisTest
{
public:
	WorkerTest();

private:
	size_t mRepeat;
};


} // namespace testing


#endif // WORKERTEST_H_INCLUDED
