#ifndef NODECALCULATORTEST_H_INCLUDED
#define NODECALCULATORTEST_H_INCLUDED


#include "TetrisTest.h"
#include "Futile/Worker.h"
#include "Futile/TypedWrapper.h"


namespace testing {


class NodeCalculatorTest : public TetrisTest
{
protected:
	FUTILE_BOX_TYPE(Depth, int);
	FUTILE_BOX_TYPE(Width, int);
	FUTILE_BOX_TYPE(WorkerCount, int);
	FUTILE_BOX_TYPE(TimeMs, int);

	void testInterrupt(Depth inDepth, Width inWidth, WorkerCount inWorkerCount, TimeMs inTimeMs);

	void testDestroy(Futile::Worker& inMainWorker, Futile::WorkerPool& inWorkerPool);
};


} // namespace testing


#endif // NODECALCULATORTEST_H_INCLUDED
