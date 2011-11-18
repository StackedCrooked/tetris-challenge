#include "test_common.hpp"

namespace {

	template <int ms>
	struct ms_per_m_shared_read {
		explicit ms_per_m_shared_read(chart_data<>& data) : data(data) {
			data.add_curve("");
			data.axes("transaction size", "time (ns)");
		}
		ms_per_m_shared_read& operator()(int shared_num) {
			uint32_t res_its = 0;
			repeat_while2 rep(ms);
			boost::barrier bar(2);
			boost::thread thr([&]() {
				std::vector<stm::shared<int>> vec(shared_num, 0);
				bar.wait();
				uint32_t its;
				for (its = 0; rep(); ++its) {
					stm::atomic([&](stm::transaction& tx) { std::for_each(vec.begin(), vec.end(), [&tx](stm::shared<int>& it) { it.open_r(tx);});});
				}
				res_its = its;
			});

			bar.wait();
			auto elapsed = rep.block();

			thr.join();
			data.insert(0, std::make_pair(shared_num, (static_cast<double>(elapsed) * 1000000.0) / static_cast<double>(res_its*shared_num)));

			return *this;
		}

		chart_data<>& data;
	};

	template <int ms>
	struct ms_per_m_shared_write {
		explicit ms_per_m_shared_write(chart_data<>& data) : data(data) {
			data.add_curve("");
			data.axes("transaction size", "time (ns)");
		}
		ms_per_m_shared_write& operator() (int shared_num) {
			uint32_t res_its = 0;
			repeat_while2 rep(ms);
			boost::barrier bar(2);
			std::vector<stm::shared<int> > vec(shared_num, 0);
			boost::thread thr([&]() {
				std::vector<stm::shared<int>> vec(shared_num, 0);
				bar.wait();
				stm::shared<int> sh(0);
				uint32_t its;
				for (its = 0; rep(); ++its) {
					stm::atomic([&](stm::transaction& tx) { std::for_each(vec.begin(), vec.end(), [&tx](stm::shared<int>& it) { it.open_rw(tx);});});
				}
				res_its = its;
			});

			bar.wait();
			auto elapsed = rep.block();

			thr.join();
			data.insert(0, std::make_pair(shared_num, (static_cast<double>(elapsed) * 1000000.0) / static_cast<double>(res_its*shared_num)));

			return *this;
		}

		chart_data<>& data;
	};
}
BOOST_AUTO_TEST_SUITE( no_contention )

	BOOST_AUTO_TEST_CASE ( empty )
{
	{	auto end = timer_timestamp() + default_test_duration * ticks_per_ms * 5;
	size_t i = 0;
	for (; timer_timestamp() < end; ++i) {
		stm::atomic([](stm::transaction&) {});
	}

	std::cout << "ns per tx: " << (default_test_duration * 5000000ull) / i << "\n";
	}

	{
		auto start = timer_timestamp();
		size_t i = 0;
		for (; i < 100000000; ++i) {
			stm::atomic([](stm::transaction&) {});
		}

		auto end = timer_timestamp();
		std::cout << "tx per ms: " << i / ((end - start) / ticks_per_ms) << "\n";

	}
}


BOOST_AUTO_TEST_CASE ( reads )
{
	chart_data<> data[iters];
	std::for_each(data, data+iters, [](chart_data<>& data) {
		ms_per_m_shared_read<default_test_duration>(data)(1)(2)(4)(8)(16)(32)(64)(128)(256)(512)(1024);
	});

	final_data<> res(data, data+iters);
	write_results("no-r.raw", print_raw(res));
	write_results("no-r.lin", make_plot(res, linplot));
	write_results("no-r.log", make_plot(res, logplot));
	write_results("no-r.loglog", make_plot(res, loglogplot));
	check_consistency();
}

BOOST_AUTO_TEST_CASE ( writes )
{
	chart_data<> data[iters];
	std::for_each(data, data+iters, [](chart_data<>& data) {
		ms_per_m_shared_write<default_test_duration>(data)(1)(2)(4)(8)(16)(32)(64)(128)(256)(512)(1024);
	});

	final_data<> res(data, data+iters);
	write_results("no-w.raw", print_raw(res));
	write_results("no-w.lin", make_plot(res, linplot));
	write_results("no-w.log", make_plot(res, logplot));
	write_results("no-w.loglog", make_plot(res, loglogplot));

	check_consistency();

}

BOOST_AUTO_TEST_SUITE_END()
