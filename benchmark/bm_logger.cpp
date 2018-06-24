/**
 * bm_logger.cpp
 * @author wherewindblow
 * @date   Dec 13, 2016
 */


#include "bm_logger.h"

#include <benchmark/benchmark.h>
#include <spdlog/spdlog.h>
#include <lights/logger.h>
#include <lights/log_sinks/file_sink.h>


#define LOGGER_FILENAME(device) device == 0 ? "/dev/null" : __func__


// --------------- General message by manual with less info ---------------

void BM_logger_spdlog_logger(benchmark::State& state)
{
	int device = state.range(0);
	auto filename = LOGGER_FILENAME(device);
	auto file_sink = std::make_shared<spdlog::sinks::simple_file_sink_st>(filename);
	auto logger = spdlog::get(filename);
	if (logger == nullptr)
	{
		logger = spdlog::create(filename, file_sink);
	}

	while (state.KeepRunning())
	{
		logger->info("{}:{}", __func__, __LINE__);
	}
}

void BM_logger_lights_TextLogger(benchmark::State& state)
{
	int device = state.range(0);
	auto file_sink = std::make_shared<lights::log_sinks::SimpleFileSink>(LOGGER_FILENAME(device));
	lights::TextLogger logger("log", file_sink);
//	logger.set_record_location(false);

	while (state.KeepRunning())
	{
		logger.info("{}:{}", __func__, __LINE__);
	}
}


// --------------- General message by automatic with more info ---------------

void BM_logger_more_lights_TextLogger(benchmark::State& state)
{
	int device = state.range(0);
	auto file_sink = std::make_shared<lights::log_sinks::SimpleFileSink>(LOGGER_FILENAME(device));
	lights::TextLogger logger("log", file_sink);

	while (state.KeepRunning())
	{
		LIGHTS_INFO(logger, "");
	}
}


void BM_logger_more_lights_BinaryLogger(benchmark::State& state)
{
	const int TEST_LOGGER = 1;

	int device = state.range(0);
	auto file_sink = std::make_shared<lights::log_sinks::SimpleFileSink>(LOGGER_FILENAME(device));
	auto str_table_ptr = lights::StringTable::create("log_str_table");
	lights::BinaryLogger logger(TEST_LOGGER, file_sink, str_table_ptr);

	while (state.KeepRunning())
	{
		LIGHTS_INFO(logger, "");
	}
}

void BM_logger()
{
#define LOGGER_BENCHMARK(func) BENCHMARK(func)->Arg(0)->Arg(1)
	LOGGER_BENCHMARK(BM_logger_spdlog_logger);
	LOGGER_BENCHMARK(BM_logger_lights_TextLogger);
	LOGGER_BENCHMARK(BM_logger_more_lights_TextLogger);
	LOGGER_BENCHMARK(BM_logger_more_lights_BinaryLogger);
}
