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
    logger.addLogHandler(boost::bind(&Print, _1));
    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
