#include <map>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>

#include <boost/thread.hpp>

#include <stm.hpp>
#include <stm/atomic_detail.hpp>

#include "common/benchmarks.hpp"
#include "common/util.hpp"

BENCHMARK("empty transactions throughput")
{
	auto func = [&](stm::atomic_detail& atm, int) { atm([](stm::transaction& tx) {}); };

	// only have one param, tx-size, it seems
	add_thread("thread", func)(1);
	set_tx_size()(1);

	run();

	// fp should generate a list of plots, where each plot is a list of 2d tuples
	//add_graph("name", "threads", "transactions/second", format::txt(), fp);
	//add_graph("name", "", "", format::plot(log, lin), fp);
	
	// where fp is either a common func or a lambda
	// say we make bm base. This func stores results

	std::cout << "results: " << results.size() << "\n";

	std::cout << "number of commits: " << count_commits(results[0][0]) << "\n";

	std::cout << "transactions/second: " << count_commits(results[0][0]) * 1000ull / 200ull << "\n";
	std::cout << "time per transaction: " << 200ull  * 1000000ull / count_commits(results[0][0]) << "ns\n";
	
}
/*
BENCHMARK("read-read contention")
{
	stm::shared<int> shds[512];
	benchmark bm;
	bm.define_thread("readers", [&](stm::atomic_detail& atm, int tx_size){ atm([&](stm::transaction& tx){ 
	        for (int i = 0; i != tx_size; ++i) { shds[i].open_r(tx);}
	    });})(0)(1)(2)(3);
	bm.define_thread("writers", [](stm::atomic_detail& atm, int tx_size){})(0)(1);
	bm.define_tx_size()(1)(2)(4)(8)(16)(32)(64)(128)(256)(512);
	bm.run();

	// pyt med output. Lige nu skal vi have en haxet måde at checke at data er korrekt
	// men den kan komme til bagefter
}
*/
