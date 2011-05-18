#include "Futile/Logger.h"
#include "CppUnit/TestRunner.h"
#include "TetrisTestSuite.h"
#include <iostream>


int RunTetrisTestSuite()
{
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
	Futile::Logger::Initializer initLogger;
    int res = RunTetrisTestSuite();
	system("pause");
    return res;    
}
