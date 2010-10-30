#include "CppUnit/TestRunner.h"
#include "TetrisCoreTestSuite.h"
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

int main(int ac, char **av)
{
    int res = RunTetrisTestSuite();
    return res;    
}

