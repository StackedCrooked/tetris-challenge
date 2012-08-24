#ifndef STM_BENCHMARK_TIMER_HPP
#define STM_BENCHMARK_TIMER_HPP 

#ifdef _MSC_VER
#include <Windows.h>
#elif defined(__MACH__)
#include <mach/mach.h>
#include <mach/mach_time.h>
#else
#include <sys/time
#endif

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


const uint64_t ticks_per_ms = timer_freq() / 1000;

inline uint64_t timestamp() throw() {
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

#endif

