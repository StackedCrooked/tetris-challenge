#ifndef STM_REGISTRAR_HPP
#define STM_REGISTRAR_HPP

#include "benchmark.hpp"

extern std::map<std::string, benchmark*> stm_benchmarks;

template <typename bm_type>
struct benchmark_registrar {
    explicit benchmark_registrar(const std::string& name) {
        stm_benchmarks[name] = new bm_type();
    }
};

#define BENCHMARK_UNIQUE_NAME2(name, line) name ## line
#define BENCHMARK_UNIQUE_NAME(name, line) BENCHMARK_UNIQUE_NAME2(name, line)
#define BENCHMARK(name) \
namespace { \
    struct BENCHMARK_UNIQUE_NAME(stm_benchmark_, __LINE__) : benchmark \
	{ \
        BENCHMARK_UNIQUE_NAME(stm_benchmark_, __LINE__)() {} \
        void benchmark_main(); \
    }; \
} \
benchmark_registrar<BENCHMARK_UNIQUE_NAME(stm_benchmark_, __LINE__)> BENCHMARK_UNIQUE_NAME(benchmark_instance, __LINE__)(name); \
void BENCHMARK_UNIQUE_NAME(stm_benchmark_, __LINE__)::benchmark_main()

#endif

