#include "Futile/Logger.h"
#include "CppUnit/TestRunner.h"
#include "TetrisTestSuite.h"
#include <boost/version.hpp>
#include <iostream>


int RunTetrisTestSuite()
{
    //BOOST_VERSION % 100 is the patch level
    //BOOST_VERSION / 100 % 1000 is the minor version
    //BOOST_VERSION / 100000 is the major version
    int major = BOOST_VERSION / 100000;
    int minor = BOOST_VERSION / 100 % 1000;
    int patch = BOOST_VERSION % 100;

    std::cout << "Boost version: " << major << "." << minor << "." << patch << std::endl;
    std::vector<std::string> args;
    args.push_back("-all");
    args.push_back("TetrisCoreTestSuite");

    CppUnit::TestRunner runner;
    runner.addTest("TetrisCoreTestSuite", TetrisCoreTestSuite::suite());
    int res = runner.run(args) ? 0 : 1;
    std::cout << "Test result: " << res << std::endl;
    return res;
}


int main(int argc, char ** argv)
{
    Futile::Logger::ScopedInitializer initLogger;
    int res = RunTetrisTestSuite();
    return res;
}
