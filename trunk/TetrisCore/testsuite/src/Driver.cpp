#include "CppUnit/TestRunner.h"
#include "TetrisCoreTestSuite.h"
#include <iostream>


int main(int ac, char **av)
{
	std::vector<std::string> args;
    args.push_back("-all");
    args.push_back("TetrisCoreTestSuite");

	CppUnit::TestRunner runner;
	runner.addTest("TetrisCoreTestSuite", TetrisCoreTestSuite::suite());
	int res = runner.run(args) ? 0 : 1;
    std::cout << "Test result: " << res << std::endl;
    std::cout << "Press ENTER to quit.";
    std::cin.get();
    return res;
}
