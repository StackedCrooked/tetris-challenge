#ifndef TEST_COMMON_HPP
#define TEST_COMMON_HPP

#ifdef _MSC_VER
#include <Windows.h>
#elif defined(__MACH__)
#include <mach/mach.h>
#include <mach/mach_time.h>
#else
#include <sys/time.h>
#endif

#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <cmath>
#include <stm.hpp>
#include <fstream>
#include <set>
#include <sstream>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <stm/atomic_detail.hpp>
#include <iostream>
#include <boost/thread/barrier.hpp>
#include <stm/debug.hpp>
#include "internal/registry.hpp"

#ifndef STM_ITERS
#define STM_ITERS 5
#endif

#ifndef STM_DURATION
#define STM_DURATION 2000
#endif

enum { test_duration = STM_DURATION, iters = STM_ITERS };

// aka ticks per second
inline uint64_t timer_freq() throw() {
#ifdef _MSC_VER
		LARGE_INTEGER tmp;
		QueryPerformanceFrequency(&tmp);
		return tmp.QuadPart;

#elif defined(__MACH__)
        mach_timebase_info_data_t timebase;
        mach_timebase_info(&timebase);
        return (1000000000ULL * timebase.denom) / timebase.numer;
#else
	return 1000000000ull;
#endif
}

namespace {
	const uint64_t ticks_per_ms = timer_freq() / 1000;
}

inline uint64_t timer_timestamp() throw() {
#ifdef _MSC_VER
		LARGE_INTEGER tmp;
		QueryPerformanceCounter(&tmp);
		return tmp.QuadPart;
#elif defined(__MACH__)
        return mach_absolute_time();
#else
		timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		return ts.tv_sec * 1000000000ull + static_cast<uint64_t>(ts.tv_nsec);
#endif
}

// simple ad-hoc helper class used for benchmarking. Simply returns a millisecond-resolution timestamp
template <typename type = double>
struct timestamp {
	type operator()() {
		return static_cast<type>(timer_timestamp()) / static_cast<type>(ticks_per_ms);
	}
};


template <int ms, typename iterator, typename tx_op>
inline int repeat_op(iterator first, iterator last, tx_op f){
	stm::atomic_detail dtl;
	timestamp<int> t;
	int start = t();
	int i;
	for (i = 0; t() - start < ms; ++i) {
		dtl([&](stm::transaction& tx){
			std::for_each(first, last, [&](stm::shared<int>& sh) {
				f(sh, tx);
			});
		});
	}
	return i;
}

template <typename x_type = int, typename y_type = double>
struct curve {
	curve() {}
	curve(std::string name) : name(name) {}
	std::string name;
	std::map<x_type, y_type> vertices;
};

template <typename x_type = int, typename y_type = double>
struct chart_data {
	chart_data() : max_x(), max_y(), is_done(false) {}

	chart_data& axes(std::string x, std::string y) {
		x_axis = x;
		y_axis = y;
		return *this;
	}
	chart_data& add_curve(std::string name) {
		curves.push_back(name);
		return *this;
	}

	void insert(int idx, std::pair<x_type, y_type> val) {
		curves[idx].vertices.insert(val);
	}

	typedef std::vector<curve<> > container;

	container& get_curves() {
		if (!is_done){
			done();
		}
		return curves;
	}

	std::string x_axis;
	std::string y_axis;

	void done() {
#ifndef __MACH__
#error blah
#endif
//		std::for_each(curves.begin(), curves.end(), [&](curve<>& c){
//			max_y = std::max(max_y, std::max_element(c.vertices.begin(), c.vertices.end(), [](std::pair<x_type, y_type> lhs, std::pair<x_type, y_type> rhs){
//				return lhs.second < rhs.second;
//			})->second);
//
//			max_x = std::max(max_x, c.vertices.rbegin()->first);
//		});
		is_done = true;
	}

	int max_x;
	double max_y;
	container curves;
	bool is_done;
};


template <typename type>
struct vertex_y {
	vertex_y(type min, type max, type median) : min(min),max(max), median(median) {}
	type max;
	type min;
	type median;
};

template <typename x_type = int, typename y_type = double>
struct final_data {
	template <typename iter>
	final_data(iter first, iter last) : curves(first->get_curves().size()), x_axis(first->x_axis), y_axis(first->y_axis) {
		// for each curve
		for (unsigned int i = 0; i < curves.size(); ++i) {
			curves[i].name = first->get_curves()[i].name;

			std::set<x_type> keys;
			// gather all the keys we need to look up
			// for each measurement
			std::for_each(first, last, [&](chart_data<x_type, y_type>& data) {
				// for each vertex
				std::transform(data.get_curves()[i].vertices.begin(), data.get_curves()[i].vertices.end(), std::inserter(keys, keys.begin()), [](std::pair<x_type, y_type> p){ return p.first;});
			});

			// for each key
			std::for_each(keys.begin(), keys.end(), [&](x_type key){

#ifndef __MACH__
#error
#endif
//				std::vector<y_type> res(last - first);
//				// gather all vertices with this key
//				std::transform(first, last, res.begin(), [&](chart_data<x_type, y_type>& data){
//					return data.get_curves()[i].vertices[key];
//				});
//				std::sort(res.begin(), res.end());
//
//				size_t size = res.size();
//				curves[i].vertices.insert(std::make_pair(key, vertex_y<y_type>(res[0], res[size-1], res[(size-1)/2])));
			});
		}
	}

	typedef x_type x_t;
	typedef vertex_y<y_type> y_t;
	std::vector<curve<x_type, vertex_y<y_type> > > curves;
	std::string x_axis;
	std::string y_axis;
};

template <typename x_type, typename y_type>
inline std::string print_raw(chart_data<x_type, y_type>& data){
	std::ostringstream str;
	std::for_each(data.get_curves().begin(),data.get_curves().end(), [&](curve<>& c){
		str << c.name << "\n";
		str << data.x_axis << "\t" << data.y_axis << "\n";
		std::for_each(c.vertices.begin(), c.vertices.end(), [&](std::pair<x_type, y_type> p) { 
			str << p.first << ":\t" << p.second << "\n";
		});
	});
	return str.str();
}

template <typename x_type, typename y_type>
inline std::string print_raw(final_data<x_type, y_type>& data){
	std::ostringstream str;
	std::for_each(data.curves.begin(),data.curves.end(), [&](curve<x_type, vertex_y<y_type> >& c){
		str << c.name << "\n";
		str << data.x_axis << "\t" << data.y_axis << "\n";
		std::for_each(c.vertices.begin(), c.vertices.end(), [&](std::pair<x_type, vertex_y<y_type> > p) { 
			str << p.first << ":\t" << p.second.median << "\t" << p.second.min << "\t" << p.second.max << "\n";
		});
	});
	return str.str();
}

struct all_plots {
	bool operator()(std::string){
		return true;
	}
};
struct select_median {
	template <typename T>
	T operator()(vertex_y<T> v){
		return v.median;
	}
};

static const char* loglogplot = "logaxis";
static const char* logplot = "semilogyaxis";
static const char* linplot = "axis";

template <typename data_type, typename plot_selector_t, typename coord_selector_t>
inline std::string make_plot(data_type& data, const char* plottype, plot_selector_t plot_fun, coord_selector_t coord_fun, bool isbar = false){
	const char* xystring = "      (%1%,%2%)\n";
	const char* legendstring = "    \\addlegendentry{%1%}\n";
	const char* barstring = "    ybar, bar width=4pt,\n";

	const char* coordsstring = 
		"    \\addplot coordinates{\n"
		"%1%" 
		"    };\n"
		"%2%";

	const char* plotstring =
		"\\begin{tikzpicture}\n"
		"  \\begin{%5%}[xlabel=%1%, ylabel=%2%,\n"
		"    height=%3%cm, width=%3%cm,\n"
		"%6%"
		"    legend style={legend pos=outer north east, cells={anchor=east}}\n"
		"]\n"
		"%4%" // addplot
		"  \\end{%5%}\n"
		"\\end{tikzpicture}\n";


	typedef typename data_type::x_t x_t;
	typedef typename data_type::y_t y_t;
	typedef curve<x_t, y_t> curve_t;

	std::ostringstream plotline;
	std::for_each(data.curves.begin(), data.curves.end(), [&](curve_t& cv){
		if (!plot_fun(cv.name)){ return; }
		std::ostringstream coordset;
		std::for_each(cv.vertices.begin(), cv.vertices.end(), [&](std::pair<x_t, y_t> p){
			coordset << boost::format(xystring) % p.first % coord_fun(p.second);
		});
		plotline << boost::format(coordsstring) % coordset.str() % (cv.name == "" ? std::string() : boost::str(boost::format(legendstring) % cv.name));
	});
	std::ostringstream finalplot;
	finalplot << boost::format(plotstring) % data.x_axis % data.y_axis % "6" % plotline.str() % plottype % (isbar ? barstring : "");
	return finalplot.str();
}
template <typename data_type>
inline std::string make_plot(data_type& data, const char* plottype = linplot, bool isbar = false){
	return make_plot(data, plottype, all_plots(), select_median(), isbar);
}

inline void write_results(std::string name, std::string results){
	std::ofstream file((name + ".txt").c_str(), std::ios::out | std::ios::trunc);
	file << results;
}

inline void check_consistency() {
	if (!stm::frontend::get_manager().is_empty()){
		std::cout << "Error: tx-local buffer not empty!" << std::endl;
	}
	std::cout << "Current version number: " << stm::debug::current_version() << std::endl;
}


// starts a timer when constructed, operator() retrieves time since object was created.
struct timer {
	timer()  : start(tm()) {}
	double operator()() { return tm() - start; }
private:
	timestamp<double> tm;
	double start;
};

struct benchmark {
    template <typename F0, typename F1>
    void operator()(F0 f0, size_t c0, F1 f1, size_t c1) {
        bool halt = false;
        std::vector<F0> f0s(c0, f0);
        std::vector<F1> f1s(c1, f1);
        
        r0s.resize(c0);
        r1s.resize(c1);
        
        boost::thread_group grp;
        boost::barrier start(c0+c1+1);
        boost::barrier end(c0+c1+1);
        
        for (size_t tid = 0; tid != c0; ++tid) {
            grp.create_thread([&]() {
                stm::atomic_detail atomic;
                
                size_t i = 0;
                start.wait();
                for (; !halt; ++i) {
                    f0s[tid](atomic);
                }
                end.wait();
                r0s[tid] = std::make_pair(i, atomic);
            });
        }
        for (size_t tid = 0; tid != c1; ++tid) {
            grp.create_thread([&]() {
                stm::atomic_detail atomic;
                
                size_t i = 0;
                start.wait();
                for (; !halt; ++i) {
                    f1s[tid](atomic);
                }
                end.wait();
                r1s[tid] = std::make_pair(i, atomic);
            });
        }
        
        start.wait();
        std::cout << "go\n";
        timer t;
        boost::this_thread::sleep(boost::posix_time::milliseconds(test_duration));
        halt = true;
        elapsed = t();
        end.wait();
        std::cout << "done\n";
        grp.join_all();
        std::cout << "joined\n";
    }
    
    typedef std::pair<size_t, stm::atomic_detail> result_type;
    std::vector<result_type> r0s;
    std::vector<result_type> r1s;
    double elapsed;
};

#endif
