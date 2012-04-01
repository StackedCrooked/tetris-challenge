#include <stm.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

namespace {
	template <int iters>
	struct writer {
		writer(boost::barrier& bar, stm::shared<int>& x) : bar(bar), x(x) {}
		void operator()(){
			bar.wait();
			for (int i = 0; i < iters; ++i){
				stm::atomic(*this);
			}
		}

		void operator()(stm::transaction& tx){
			int& i = x.open_rw(tx);
			++i;
		}

		boost::barrier& bar;
		stm::shared<int>& x;
	};

	template <int iters>
	struct reader {
		reader(boost::barrier& bar, stm::shared<int>& x) : bar(bar), x(x) {}
		void operator()(){
			bar.wait();
			for (int i = 0; i < iters; ++i){
				stm::atomic(*this);
			}
		}

		void operator()(stm::transaction& tx){
			x.open_r(tx);
		}

		boost::barrier& bar;
		stm::shared<int>& x;
	};
}
BOOST_AUTO_TEST_SUITE( race_conds )

BOOST_AUTO_TEST_CASE( race_cond_simple )
{
	enum { iterations = 5000 };
	stm::shared<int> x(0);
	boost::barrier bar(2);
	boost::thread_group gr;
	gr.create_thread(writer<iterations>(bar, x));
	gr.create_thread(reader<iterations>(bar, x));

	gr.join_all();
}

BOOST_AUTO_TEST_SUITE_END()
