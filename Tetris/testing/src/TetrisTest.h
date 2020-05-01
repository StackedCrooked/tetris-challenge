#ifndef TETRISCORETESTSUITE_H_INCLUDED
#define TETRISCORETESTSUITE_H_INCLUDED


#include <gtest/gtest.h>


namespace testing {


std::string GetEnv(const std::string& name, const std::string& def = "");


class TetrisTest : public Test
{
public:
    virtual ~TetrisTest() {}

    static void BeBusy();

protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};


} // namespace testing


#endif // TETRISCORETESTSUITE_H_INCLUDED
