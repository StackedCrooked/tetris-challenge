#include "test_common.hpp"
//
//namespace {
//
//	template <int ms>
//	struct ms_per_m_shared_read {
//		explicit ms_per_m_shared_read(chart_data<>& data) : data(data) {
//			data.add_curve("");
//			data.axes("transaction size", "time (ns)");
//		}
//		ms_per_m_shared_read& operator()(int shared_num) {
//			uint32_t res_its = 0;
//			repeat_while2 rep(ms);
//			boost::barrier bar(2);
//			boost::thread thr([&]() {
//				std::vector<stm::shared<int>> vec(shared_num, 0);
//				bar.wait();
//				uint32_t its;
//				for (its = 0; rep(); ++its) {
//					stm::atomic([&](stm::transaction& tx) { std::for_each(vec.begin(), vec.end(), [&tx](stm::shared<int>& it) { it.open_r(tx);});});
//				}
//				res_its = its;
//			});
//
//			bar.wait();
//			auto elapsed = rep.block();
//
//			thr.join();
//			data.insert(0, std::make_pair(shared_num, (static_cast<double>(elapsed) * 1000000.0) / static_cast<double>(res_its*shared_num)));
//
//			return *this;
//		}
//
//		chart_data<>& data;
//	};
//
//	template <int ms>
//	struct ms_per_m_shared_write {
//		explicit ms_per_m_shared_write(chart_data<>& data) : data(data) {
//			data.add_curve("");
//			data.axes("transaction size", "time (ns)");
//		}
//		ms_per_m_shared_write& operator() (int shared_num) {
//			uint32_t res_its = 0;
//			repeat_while2 rep(ms);
//			boost::barrier bar(2);
//			std::vector<stm::shared<int> > vec(shared_num, 0);
//			boost::thread thr([&]() {
//				std::vector<stm::shared<int>> vec(shared_num, 0);
//				bar.wait();
//				stm::shared<int> sh(0);
//				uint32_t its;
//				for (its = 0; rep(); ++its) {
//					stm::atomic([&](stm::transaction& tx) { std::for_each(vec.begin(), vec.end(), [&tx](stm::shared<int>& it) { it.open_rw(tx);});});
//				}
//				res_its = its;
//			});
//
//			bar.wait();
//			auto elapsed = rep.block();
//
//			thr.join();
//			data.insert(0, std::make_pair(shared_num, (static_cast<double>(elapsed) * 1000000.0) / static_cast<double>(res_its*shared_num)));
//
//			return *this;
//		}
//
//		chart_data<>& data;
//	};

	void empty()
	{
		auto start = timer_timestamp();
		size_t i = 0;
		for (; i < 10000000; ++i) {
			stm::atomic([](stm::transaction&) {});
		}

		auto end = timer_timestamp();
		uint64_t ns_per_tx = ((end - start) * 1000000) / (ticks_per_ms *  static_cast<uint64_t>(i));
		uint64_t tx_per_ms = (static_cast<uint64_t>(i) * ticks_per_ms) / (end - start);

		std::cout << "ns per tx: " << ns_per_tx << "\n";
		std::cout << "tx per ms: " << tx_per_ms << "\n";
	}


//	void reads()
//	{
//		chart_data<> data[iters];
//		std::for_each(data, data+iters, [](chart_data<>& data) {
//			ms_per_m_shared_read<default_test_duration>(data)(1)(2)(4)(8)(16)(32)(64)(128)(256)(512)(1024);
//		});
//
//		final_data<> res(data, data+iters);
//		write_results("no-r.raw", print_raw(res));
//		write_results("no-r.lin", make_plot(res, linplot));
//		write_results("no-r.log", make_plot(res, logplot));
//		write_results("no-r.loglog", make_plot(res, loglogplot));
//		check_consistency();
//	}
//
//	void writes()
//	{
//		chart_data<> data[iters];
//		std::for_each(data, data+iters, [](chart_data<>& data) {
//			ms_per_m_shared_write<default_test_duration>(data)(1)(2)(4)(8)(16)(32)(64)(128)(256)(512)(1024);
//		});
//
//		final_data<> res(data, data+iters);
//		write_results("no-w.raw", print_raw(res));
//		write_results("no-w.lin", make_plot(res, linplot));
//		write_results("no-w.log", make_plot(res, logplot));
//		write_results("no-w.loglog", make_plot(res, loglogplot));
//
//		check_consistency();
//	}
//}
//
STM_BENCHMARK( no_contention )
{
	empty();
//	reads();
//	writes();
}