#ifndef NODECALCULATORTEST_H_INCLUDED
#define NODECALCULATORTEST_H_INCLUDED


#include "TetrisTest.h"
#include "Futile/Worker.h"
#include "Futile/TypedWrapper.h"


namespace testing {


class NodeCalculatorTest : public TetrisTest
{
protected:
	Futile_TypedWrapper(Depth, int);
	Futile_TypedWrapper(Width, int);
	Futile_TypedWrapper(WorkerCount, int);
	Futile_TypedWrapper(TimeMs, int);

	void testInterrupt(Depth inDepth, Width inWidth, WorkerCount inWorkerCount, TimeMs inTimeMs);

	void testDestroy(Worker & inMainWorker, WorkerPool & inWorkerPool);
};


} // namespace testing


#endif // NODECALCULATORTEST_H_INCLUDED
