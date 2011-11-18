#include "test_common.hpp"

namespace {
	template <int ms_duration, int shared_num>
	struct rw_ops_per_second {
		explicit rw_ops_per_second(chart_data<int, int>& data) : data(data) {
			data.add_curve("writer").add_curve("reader");
			data.axes("threads", "commits/sec");
		}
		rw_ops_per_second& operator()(int reader_num){
			int reads, writes;
			std::vector<stm::shared<int> > val(shared_num, 0);
			boost::barrier bar(reader_num+2);
			boost::thread_group grp;
			std::vector<int> ops(reader_num, 0);
			repeat_while2 rep(ms_duration);
			for (int i = 0; i < reader_num; ++i) {
				grp.create_thread([&bar, &val, &ops, &rep, i](){
					stm::atomic_detail atm;
					bar.wait();
					uint32_t its = 0;
					for (its = 0; rep(); ++its) {
						//for (int j = 0; j < 20000000; ++j){
						atm([&](stm::transaction& tx) -> void {
							std::for_each(val.begin(), val.end(), [&tx](stm::shared<int>& v) {v.open_r(tx); });
						});
					}
					ops[i] = its;
				});
			}
			grp.create_thread([&](){
				stm::atomic_detail atm;
				bar.wait();
				uint32_t its = 0;
				for (its = 0; rep(); ++its) {
					//for (int j = 0; j < 20000000; ++j){
					atm([&](stm::transaction& tx) {
						std::for_each(val.begin(), val.end(), [&tx](stm::shared<int>& v) {v.open_rw(tx); });
					});
				} 
				writes = its;
			});
			bar.wait();
			auto elapsed = rep.block();

			grp.join_all();
			reads = std::accumulate(ops.begin(), ops.end(), 0);
			data.insert(0, std::make_pair(reader_num, static_cast<int>((static_cast<double>(writes) * 1000.0) / static_cast<double>(elapsed))));
			data.insert(1, std::make_pair(reader_num, static_cast<int>((static_cast<double>(reads) * 1000.0) / static_cast<double>(elapsed))));

			return *this;
		}

		chart_data<int, int>& data;
	};

	template <int ms_duration, int shared_num>
	struct rw_stats {
		explicit rw_stats(chart_data<int, int>& data) : data(data) {
			data.add_curve("commits (read)").add_curve("rollbacks (read)").add_curve("ropen").add_curve("rlock").add_curve("rvalidate");
			data.add_curve("commits (write)").add_curve("rollbacks (write)").add_curve("wopen").add_curve("wlock").add_curve("wvalidate");
			data.axes("threads", "ops/sec");
		}
		rw_stats& operator()(int thread_num){
			std::vector<stm::shared<int> > val(shared_num, 0);
			boost::barrier bar(thread_num+2);
			boost::thread_group grp;
			std::vector<stm::atomic_detail> read_stats(thread_num);
			stm::atomic_detail write_stats;
			repeat_while2 rep(ms_duration);
			for (int i = 0; i < thread_num; ++i) {
				grp.create_thread([&bar, &val, &read_stats, &rep, i](){
					bar.wait();
					stm::atomic_detail atm;
					while(rep()) {
						atm([&](stm::transaction& tx) {
							std::for_each(val.begin(), val.end(), [&tx](stm::shared<int>& v) {v.open_r(tx); });
						});
					}
					read_stats[i] = atm;
				});
			}
			grp.create_thread([&](){
				bar.wait();
				stm::atomic_detail atm;
				while(rep()) {
					atm([&](stm::transaction& tx) {
						std::for_each(val.begin(), val.end(), [&tx](stm::shared<int>& v) {v.open_rw(tx); });
					});
				} 
				write_stats = atm;
			});

			bar.wait();
			auto elapsed = rep.block();

			grp.join_all();
			stm::atomic_detail res = std::accumulate(read_stats.begin(), read_stats.end(), stm::atomic_detail(), [](stm::atomic_detail lhs, stm::atomic_detail rhs)->stm::atomic_detail{
				lhs.commits += rhs.commits;
				lhs.open_failures += rhs.open_failures;
				lhs.commit_lock_failures += rhs.commit_lock_failures;
				lhs.commit_validate_failures += rhs.commit_validate_failures;
				return lhs;
			});

			data.insert(0, std::make_pair(thread_num, (res.commits * 1000) / elapsed));
			data.insert(1, std::make_pair(thread_num, ((res.open_failures + res.commit_lock_failures + res.commit_validate_failures) * 1000) / elapsed));
			data.insert(2, std::make_pair(thread_num, (res.open_failures * 1000) / elapsed));
			data.insert(3, std::make_pair(thread_num, (res.commit_lock_failures * 1000) / elapsed));
			data.insert(4, std::make_pair(thread_num, (res.commit_validate_failures * 1000) / elapsed));

			data.insert(5, std::make_pair(thread_num, (write_stats.commits * 1000) / elapsed));
			data.insert(6, std::make_pair(thread_num, ((write_stats.open_failures + write_stats.commit_lock_failures + write_stats.commit_validate_failures) * 1000) / elapsed));
			data.insert(7, std::make_pair(thread_num, (write_stats.open_failures * 1000) / elapsed));
			data.insert(8, std::make_pair(thread_num, (write_stats.commit_lock_failures * 1000) / elapsed));
			data.insert(9, std::make_pair(thread_num, (write_stats.commit_validate_failures * 1000) / elapsed));

			return *this;
		}

		chart_data<int, int>& data;
	};

	template <int ms_duration, int shared_num>
	struct rw_timings {
		rw_timings(chart_data<int, int>& data, int thread_num) : data(data), thread_num(thread_num) {
			data.add_curve("read").add_curve("write");
			data.axes("ms", "transactions");
		}
		rw_timings& operator()(){
			std::vector<stm::shared<int> > val(shared_num, 0);
			boost::barrier bar(thread_num+2);
			boost::thread_group grp;
			std::vector<std::map<unsigned int, int>> read_stats(thread_num);
			std::map<unsigned int, int> write_stats;
			repeat_while2 rep(ms_duration);
			for (int i = 0; i < thread_num; ++i) {
				grp.create_thread([&, i](){
					std::map<unsigned int, int> timings;
					bar.wait();
					while(rep()) {
						timer tm;
						stm::atomic([&](stm::transaction& tx) {
							std::for_each(val.begin(), val.end(), [&tx](stm::shared<int>& v) {v.open_r(tx); });
						});
						auto elapsed = static_cast<int>(std::floor(tm() + 0.5f));
						++timings[(elapsed / 2)*2];
					}
					read_stats[i] = timings;
				});
			}
			grp.create_thread([&](){
				std::map<unsigned int, int> timings;
				bar.wait();
				while(rep()) {
					timer tm;
					stm::atomic([&](stm::transaction& tx) {
						std::for_each(val.begin(), val.end(), [&tx](stm::shared<int>& v) {v.open_rw(tx); });
					});
					auto elapsed = static_cast<int>(std::floor(tm() + 0.5f));
					++timings[(elapsed / 2)*2];
				} 
				write_stats = timings;
			});

			bar.wait();
			rep.block();

			grp.join_all();
			auto res = std::accumulate(read_stats.begin(), read_stats.end(), std::map<unsigned int, int>(), [](std::map<unsigned int, int> lhs, const std::map<unsigned int, int>& rhs)-> std::map<unsigned int, int> {
				std::for_each(rhs.begin(), rhs.end(), [&](std::pair<unsigned int, int> p){
					lhs[p.first] += p.second;
				});
				return lhs;
			});


			std::for_each(res.begin(), res.end(), [this](std::pair<unsigned int, int> p){
				data.insert(0, p);
			});
			std::for_each(write_stats.begin(), write_stats.end(), [this](std::pair<unsigned int, int> p){
				data.insert(1, p);
			});

			return *this;
		}

		chart_data<int, int>& data;
		int thread_num;
	};

}

BOOST_AUTO_TEST_SUITE( rw_contention )

	BOOST_AUTO_TEST_CASE ( basic_s )
{
	chart_data<int, int> data[iters];
	std::for_each(data, data+iters, [](chart_data<int, int>& data) {
		rw_ops_per_second<default_test_duration, 1>(data)(1)(2)(3);
	});

	final_data<int, int> res(data, data+iters);
	write_results("rw-basic-s.raw", print_raw(res));
	write_results("rw-basic-s.lin", make_plot(res, linplot));
	write_results("rw-basic-s.log", make_plot(res, logplot));
	check_consistency();
}

BOOST_AUTO_TEST_CASE ( basic_l )
{
	chart_data<int, int> data[iters];
	std::for_each(data, data+iters, [](chart_data<int, int>& data) {
		rw_ops_per_second<default_test_duration, big_tx>(data)(1)(2)(3);
	});

	final_data<int, int> res(data, data+iters);
	write_results("rw-basic-l.raw", print_raw(res));
	write_results("rw-basic-l.lin", make_plot(res, linplot));
	write_results("rw-basic-l.log", make_plot(res, logplot));
	check_consistency();
}

BOOST_AUTO_TEST_CASE ( stats_s )
{
	chart_data<int, int> data[iters];
	std::for_each(data, data+iters, [](chart_data<int, int>& data) {
		rw_stats<default_test_duration, 1>(data)(1)(2)(3);
	});

	final_data<int, int> res(data, data+iters);
	write_results("rw-stats-s.raw", print_raw(res));
	write_results("rw-stats-s.log", make_plot(res, logplot, [](std::string name){
		return name == "commits (read)" || name == "rollbacks (read)" || name == "commits (write)" || name == "rollbacks (write)";  
	}, select_median()));
	write_results("rw-stats-s.lin", make_plot(res, linplot, [](std::string name){
		return name == "commits (read)" || name == "rollbacks (read)" || name == "commits (write)" || name == "rollbacks (write)";  
	}, select_median()));
	check_consistency();
}
BOOST_AUTO_TEST_CASE ( stats_l )
{
	chart_data<int, int> data[iters];
	std::for_each(data, data+iters, [](chart_data<int, int>& data) {
		rw_stats<default_test_duration, big_tx>(data)(1)(2)(3);
	});

	final_data<int, int> res(data, data+iters);
	write_results("rw-stats-l.raw", print_raw(res));
	write_results("rw-stats-l.log", make_plot(res, logplot, [](std::string name){
		return name == "commits (read)" || name == "rollbacks (read)" || name == "commits (write)" || name == "rollbacks (write)";  
	}, select_median()));
	write_results("rw-stats-l.lin", make_plot(res, linplot, [](std::string name){
		return name == "commits (read)" || name == "rollbacks (read)" || name == "commits (write)" || name == "rollbacks (write)";  
	}, select_median()));
	check_consistency();
}

BOOST_AUTO_TEST_CASE ( hist_s )
{
	chart_data<int, int> data[iters];
	std::for_each(data, data+iters, [](chart_data<int, int>& data) {
		rw_timings<default_test_duration*2, 1>(data, 3)();
	});

	final_data<int, int> res(data, data+iters);
	write_results("rw-hist-s.raw", print_raw(res));
	write_results("rw-hist-s.log", make_plot(res, logplot, true));
	write_results("rw-hist-s.lin", make_plot(res, linplot, true));
	check_consistency();
}

BOOST_AUTO_TEST_CASE ( hist_l )
{
	chart_data<int, int> data[iters];
	std::for_each(data, data+iters, [](chart_data<int, int>& data) {
		rw_timings<default_test_duration*2, big_tx>(data, 3)();
	});

	final_data<int, int> res(data, data+iters);
	write_results("rw-hist-l.raw", print_raw(res));
	write_results("rw-hist-l.log", make_plot(res, logplot, true));
	write_results("rw-hist-l.lin", make_plot(res, linplot, true));
	check_consistency();
}

BOOST_AUTO_TEST_SUITE_END()
