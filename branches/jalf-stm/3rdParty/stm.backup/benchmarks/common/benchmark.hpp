#ifndef STM_BENCHMARK_HPP
#define STM_BENCHMARK_HPP

#include "timer.hpp"
#include "result.hpp"
#include "params.hpp"

#include <stm/atomic_detail.hpp>

#include <boost/thread.hpp>
#include <functional>
#include <string>
#include <map>

struct benchmark {
    inserter set_tx_size(){
        return inserter(parameters[tx_size_param_name].vals);
    }
    inserter add_thread(std::string key, thread_func func){
        parameters[key] = param_data(func);
        return inserter(parameters[key].vals);
    }

	virtual void benchmark_main() = 0;

    void run() {

    for (int iter = 0; iter != iter_count; ++iter) {
    results.resize(results.size() + 1);
    auto& current_iter = *results.rbegin();

    std::vector<param_pos> active_params;

    // ensure that tx_size is defined (no-op if it is)
    if (parameters[tx_size_param_name].vals.size() == 0){
        parameters[tx_size_param_name].vals.push_back(0);
    }

    // for each parameter, track where we've gotten to. iow, an iter pointing to param, and an iter pointing to its current val
        std::transform(parameters.begin(), parameters.end(), std::back_inserter(active_params), [](full_param_type& val){
        return param_pos(val);
    });

    for(int i = 0;;++i) {
//      std::cout << "running iteration " << i << '\n';
        // run benchmark with these params
        // count number of threads
        int n = std::accumulate(active_params.begin(), active_params.end(), 0, [](int sum, const param_pos& val){
            return (val.data().func) ? sum + *val.current_val : sum;
        });
        int tx_size = *std::find_if(active_params.begin(), active_params.end(), [](const param_pos& val){ return !val.data().func; })->current_val;

        current_iter.resize(current_iter.size() + 1);
        auto& current_permutation = *current_iter.rbegin();
        current_permutation.tx_size = tx_size;

        boost::barrier bar(n+1);
        bool stop = false;

        // now, create all the threads
        boost::thread_group grp;
        std::for_each(active_params.begin(), active_params.end(), [&](param_pos& val) {
        if (val.data().func) {
            current_permutation.threads[val.name()].reserve(*val.current_val);
            for (int i = 0; i < *val.current_val; ++i) {
            current_permutation.threads[val.name()].push_back(stm::atomic_detail());
            stm::atomic_detail& atm = *current_permutation.threads[val.name()].rbegin();
            grp.create_thread([&](){
                bar.wait();
                // wrap this in a timer loop
                stm::atomic_detail dtl;
                while (!stop) {
                    val.data().func(dtl, tx_size);
                }
                atm = dtl;
            });
            }
            }
        });

        // now, enter the barrier, sleep, and then enter barrier again
        bar.wait();
		// start timer
		auto start_time = timestamp();
        boost::this_thread::sleep(boost::posix_time::milliseconds(duration_ms));
        stop = true;
        grp.join_all();
		// stop timer
		auto stop_time = timestamp();
		std::cout << "elapsed ms: " << (stop_time - start_time) / ticks_per_ms << "\n";
        // accumulate results

        // find the first param which isn't at max
        auto param_it = std::find_if(active_params.begin(), active_params.end(), [](param_pos& val){ return std::prev(val.data().vals.end()) != val.current_val; });
        // if none found, break
        if (param_it == active_params.end()) { break; }
        // increment it
        ++param_it->current_val;
        // reset all before it
        std::for_each(active_params.begin(), param_it, [](param_pos& val){ val.current_val = val.data().vals.begin(); });
    }
    }
    }

//private:

    // when running, iterate through all params. The ones where function isn't null are threads
    std::map<std::string, param_data> parameters;
    result_type results;
};

#endif

