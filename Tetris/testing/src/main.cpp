#include "Futile/Logger.h"
#include <gtest/gtest.h>
#include <iostream>


using testing::Test;
using testing::InitGoogleTest;
using namespace Futile;


void Print(const std::string & msg)
{
    std::cout << msg << std::endl;
}


int main(int argc, char **argv)
{
    Logger::ScopedInitializer initLogger;
    Logger & logger = Logger::Instance();
    logger.setLogHandler(Print);
    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
