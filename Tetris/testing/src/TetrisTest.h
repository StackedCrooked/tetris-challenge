#ifndef TETRISCORETESTSUITE_H_INCLUDED
#define TETRISCORETESTSUITE_H_INCLUDED


#include "Poco/Stopwatch.h"
#include "Poco/Types.h"
#include <gtest/gtest.h>


namespace Futile {}
namespace Tetris {}


namespace testing {


using namespace Futile;
using namespace Tetris;


class TetrisTest : public Test
{
public:
    virtual ~TetrisTest() {}

    static void BeBusy();

protected:
    virtual void SetUp() {}
    virtual void TearDown() {}

    Poco::Stopwatch mStopwatch;
};


} // namespace testing


#endif // TETRISCORETESTSUITE_H_INCLUDED
