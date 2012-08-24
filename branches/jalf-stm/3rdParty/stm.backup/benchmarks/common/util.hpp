#ifndef STM_BENCHMARK_UTILS_HPP
#define STM_BENCHMARK_UTILS_HPP

#include "benchmarks.hpp"
#include <stm/atomic_detail.hpp>

template <typename It, typename F>
auto median(It first, It last, F f) -> decltype(*first) {
    std::sort(first, last);
    int dist = std::distance(first, last);
    return *std::next(first, dist/2);
}

typedef std::pair<std::string, std::vector<stm::atomic_detail>> thread_result;

std::vector<stm::atomic_detail> filter(const single_permutation_result& res, const std::string& match) {
	std::vector<thread_result> tmp1;
	std::copy_if(res.threads.begin(), res.threads.end(), std::back_inserter(tmp1), [&](const thread_result& res){ 
		return match == "" || match == res.first;
	});
	std::vector<stm::atomic_detail> tmp;
	std::for_each(tmp1.begin(), tmp1.end(), [&](const thread_result& res) {
		std::copy(res.second.begin(), res.second.end(), std::back_inserter(tmp));
	});
	return tmp;
}

int count_commits(const single_permutation_result& res, const std::string& threads = "") {
	auto data = filter(res, threads);
	return std::accumulate(data.begin(), data.end(), 0, [](int acc, stm::atomic_detail atm){ return acc + atm.commits; });
}

int count_threads(const single_permutation_result& res, const std::string& threads = "") {
	auto data = filter(res, threads);
	return std::accumulate(data.begin(), data.end(), 0, [](int acc, stm::atomic_detail atm){ return acc + 1; });
}

#endif

