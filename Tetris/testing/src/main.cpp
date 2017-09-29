#include <iostream>
#include "gtest/gtest.h"


TEST(TetrisTest, Benchmark)
{
    ASSERT_TRUE(true);
}


TEST(TetrisTest, ComputerPlayerTest)
{
    ASSERT_TRUE(true);
}


TEST(TetrisTest, NodeCalculatorTest)
{
    ASSERT_TRUE(true);
}


TEST(TetrisTest, STMTest)
{
    ASSERT_TRUE(true);
}


TEST(TetrisTest, TetrisTest)
{
    ASSERT_TRUE(true);
}


TEST(TetrisTest, WorkerPoolTest)
{
    ASSERT_TRUE(true);
}


TEST(TetrisTest, WorkerTest)
{
    ASSERT_TRUE(true);
}


int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}



#if 0
#include "Futile/Logger.h"
#include "Futile/MainThreadImpl.h"
#include <gtest/gtest.h>
#include <iostream>


using testing::Test;
using testing::InitGoogleTest;
using namespace Futile;


namespace Futile {


std::unique_ptr<MainThreadImpl> CreateMainThreadImpl()
{
    throw std::logic_error("Tetris unit tests should not require a message loop.");
}


} // namespace Futile


void Print(const std::string & msg)
{
    std::cout << msg << std::endl;
}


int main(int argc, char **argv)
{
    Logger::ScopedInitializer initLogger;
    Logger & logger = Logger::Instance();
    logger.addLogHandler(Print);
    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#endif
