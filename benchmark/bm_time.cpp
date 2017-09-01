/**
 * bm_time.cpp
 * @author wherewindblow
 * @date   Dec 12, 2016
 */


#include "bm_time.h"

#include <ctime>
#include <chrono>

#include <sys/time.h>
#include <benchmark/benchmark.h>
#include <lights/logger.h>


void BM_time_std_time(benchmark::State& state)
{
	int is_format_time = state.range(0);
	while (state.KeepRunning())
	{
		std::time_t time = std::time(nullptr);

		if (is_format_time)
		{
			std::ctime(&time);
		}
	}
}


void BM_time_std_chrono(benchmark::State& state)
{
	int is_format_time = state.range(0);
	while (state.KeepRunning())
	{
		namespace chrono = std::chrono;
		auto time_point = chrono::system_clock::now();

		if (is_format_time)
		{
			auto t = chrono::system_clock::to_time_t(time_point);
			std::ctime(&t);
		}
	}
}


void BM_time_gettimeofday(benchmark::State& state)
{
	int is_format_time = state.range(0);
	while (state.KeepRunning())
	{
		::timeval tv;
		::gettimeofday(&tv, nullptr);

		if (is_format_time)
		{
			std::ctime(&tv.tv_sec);
		}
	}
}


void BM_time_clock_gettime(benchmark::State& state)
{
	int is_format_time = state.range(0);
	while (state.KeepRunning())
	{
		::timespec ts;
		::clock_gettime(CLOCK_REALTIME, &ts);

		if (is_format_time)
		{
			std::ctime(&ts.tv_sec);
		}
	}
}


void BM_time_lights_PreciseTime(benchmark::State& state)
{
	int is_format_time = state.range(0);
	while (state.KeepRunning())
	{
		auto time = lights::get_precise_time();

		if (is_format_time)
		{
			std::ctime(&time.seconds);
		}
	}
}


void BM_time()
{
#define TIME_BENCHMARK(func) BENCHMARK(func)->Arg(0)->Arg(1)
	TIME_BENCHMARK(BM_time_std_time);
	TIME_BENCHMARK(BM_time_std_chrono);
	TIME_BENCHMARK(BM_time_gettimeofday);
	TIME_BENCHMARK(BM_time_clock_gettime);
	TIME_BENCHMARK(BM_time_lights_PreciseTime);
}
