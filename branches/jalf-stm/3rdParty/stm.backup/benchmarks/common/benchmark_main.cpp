#include "benchmarks.hpp"
#include <algorithm>
#include <iostream>

std::map<std::string, benchmark*> stm_benchmarks;

int main() {
	std::for_each(stm_benchmarks.begin(), stm_benchmarks.end(), [](std::pair<std::string, benchmark*> p){
		std::cout << p.first << ":\n";

		p.second->benchmark_main();
		delete p.second;
	});
}

