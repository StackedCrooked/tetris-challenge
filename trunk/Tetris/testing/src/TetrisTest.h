#ifndef TETRISCORETESTSUITE_H_INCLUDED
#define TETRISCORETESTSUITE_H_INCLUDED


#include "Poco/Stopwatch.h"
#include <gtest/gtest.h>


class TetrisTest : public testing::Test
{
public:
    virtual ~TetrisTest() {}

    static void BeBusy();

protected:
    virtual void SetUp() {}
    virtual void TearDown() {}

    Poco::Stopwatch mStopwatch;
};


#endif // TETRISCORETESTSUITE_H_INCLUDED
