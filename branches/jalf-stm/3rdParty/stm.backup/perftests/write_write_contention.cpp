#include "test_common.hpp"
//
//namespace {
//	template <int ms_duration, int shared_num>
//	struct ww_ops_per_second {
//		explicit ww_ops_per_second(chart_data<int, int64_t>& data) : data(data) {
//			data.add_curve("");
//			data.axes("threads", "commits/sec");
//		}
//		ww_ops_per_second& operator()(int thread_num){
//			timestamp<int> t;
//			std::vector<stm::shared<int> > val(shared_num, 0);
//			boost::barrier bar(thread_num+1);
//			boost::thread_group grp;
//			std::vector<int64_t> ops(thread_num, 0);
//			repeat_while2 rep(ms_duration);
//			for (int i = 0; i < thread_num; ++i) {
//				grp.create_thread([&bar, &val, &ops, &t, &rep, i](){
//					bar.wait();
//					stm::atomic_detail atm;
//					uint32_t its = 0;
//					for (its = 0; rep(); ++its){
//						atm([&](stm::transaction& tx) {
//							std::for_each(val.begin(), val.end(), [&tx](stm::shared<int>& v) {v.open_rw(tx); });;
//						});
//					}
//					ops[i] = its;
//				});
//			}
//
//			bar.wait();
//			auto elapsed = rep.block();
//
//			grp.join_all();
//			int64_t writes = std::accumulate(ops.begin(), ops.end(), 0);
//			data.insert(0, std::make_pair(thread_num, static_cast<int>((static_cast<double>(writes) * 1000.0) / static_cast<double>(elapsed))));
//			return *this;
//		}
//
//		chart_data<int, int64_t>& data;
//	};
//
//	template <int ms_duration, int shared_num>
//	struct ww_stats {
//		explicit ww_stats(chart_data<int, int>& data) : data(data) {
//			data.add_curve("commits").add_curve("rollbacks").add_curve("open").add_curve("lock").add_curve("validate");
//			data.axes("threads", "ops/sec");
//		}
//		ww_stats& operator()(int thread_num){
//			timestamp<int> t;
//			std::vector<stm::shared<int> > val(shared_num, 0);
//			boost::barrier bar(thread_num+1);
//			boost::thread_group grp;
//			std::vector<stm::atomic_detail> stats(thread_num);
//			repeat_while2 rep(ms_duration);
//			for (int i = 0; i < thread_num; ++i) {
//				grp.create_thread([&bar, &val, &stats, &t, &rep, i](){
//					bar.wait();
//					stm::atomic_detail atm;
//					uint32_t its = 0;
//					for (; rep(); ++its){
//						atm([&](stm::transaction& tx) {
//							std::for_each(val.begin(), val.end(), [&tx](stm::shared<int>& v) {v.open_rw(tx); });
//						});
//					}
//					stats[i] = atm;
//				});
//			}
//
//			bar.wait();
//			auto elapsed = rep.block();
//
//			grp.join_all();
//			stm::atomic_detail res = std::accumulate(stats.begin(), stats.end(), stm::atomic_detail(), [](stm::atomic_detail lhs, stm::atomic_detail rhs)->stm::atomic_detail{
//				lhs.commits += rhs.commits;
//				lhs.open_failures += rhs.open_failures;
//				lhs.commit_lock_failures += rhs.commit_lock_failures;
//				lhs.commit_validate_failures += rhs.commit_validate_failures;
//				return lhs;
//			});
//			data.insert(0, std::make_pair(thread_num, static_cast<int>((static_cast<double>(res.commits) * 1000.0) / static_cast<double>(elapsed))));
//			data.insert(1, std::make_pair(thread_num, static_cast<int>((static_cast<double>(res.open_failures + res.commit_lock_failures + res.commit_validate_failures) * 1000.0) / static_cast<double>(elapsed))));
//			data.insert(2, std::make_pair(thread_num, static_cast<int>((static_cast<double>(res.open_failures) * 1000.0) / static_cast<double>(elapsed))));
//			data.insert(3, std::make_pair(thread_num, static_cast<int>((static_cast<double>(res.commit_lock_failures) * 1000.0) / static_cast<double>(elapsed))));
//			data.insert(4, std::make_pair(thread_num, static_cast<int>((static_cast<double>(res.commit_validate_failures) * 1000.0) / static_cast<double>(elapsed))));
//
//			return *this;
//		}
//
//		chart_data<int, int>& data;
//	};
//
//	void basic_s()
//	{
//		chart_data<int, int64_t> data[iters];
//		std::for_each(data, data+iters, [](chart_data<int, int64_t>& data) {
//			ww_ops_per_second<default_test_duration*10, 1>(data)(2)(3)(4);
//		});
//
//		final_data<int, int64_t> res(data, data+iters);
//		write_results("ww-basic-s.raw", print_raw(res));
//		write_results("ww-basic-s.log", make_plot(res, logplot));
//		write_results("ww-basic-s.lin", make_plot(res, linplot));
//		check_consistency();
//	}
//
//	void basic_l()
//	{
//		chart_data<int, int64_t> data[iters];
//		std::for_each(data, data+iters, [](chart_data<int, int64_t>& data) {
//			ww_ops_per_second<default_test_duration, big_tx>(data)(2)(3)(4);
//		});
//
//		final_data<int, int64_t> res(data, data+iters);
//		write_results("ww-basic-l.raw", print_raw(res));
//		write_results("ww-basic-l.log", make_plot(res, logplot));
//		write_results("ww-basic-l.lin", make_plot(res, linplot));
//		check_consistency();
//	}
//
//	void stats_s()
//	{
//		chart_data<int, int> data[iters];
//		std::for_each(data, data+iters, [](chart_data<int, int>& data) {
//			ww_stats<default_test_duration, 1>(data)(2)(3)(4);
//		});
//
//		final_data<int, int> res(data, data+iters);
//		write_results("ww-stats-s.raw", print_raw(res));
//		write_results("ww-stats-s.log", make_plot(res, logplot));
//		write_results("ww-stats-s.lin", make_plot(res, linplot));
//		check_consistency();
//	}
//
//	void stats_l()
//	{
//		chart_data<int, int> data[iters];
//		std::for_each(data, data+iters, [](chart_data<int, int>& data) {
//			ww_stats<default_test_duration, big_tx>(data)(2)(3)(4);
//		});
//
//		final_data<int, int> res(data, data+iters);
//		write_results("ww-stats-l.raw", print_raw(res));
//		write_results("ww-stats-l.log", make_plot(res, logplot));
//		write_results("ww-stats-l.lin", make_plot(res, linplot));
//		check_consistency();
//	}
//}


STM_BENCHMARK(write_write)
{
//	basic_s();
//	basic_l();
//	stats_s();
//	stats_l();
}

