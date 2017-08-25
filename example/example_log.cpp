/**
 * example_log.cpp
 * @author wherewindblow
 * @date   Aug 23, 2017
 */

#include "example_log.h"

#include <lights/logger.h>
#include <lights/log_sinks/file_sink.h>
#include <lights/log_sinks/stdout_sink.h>


namespace lights {
namespace example {

namespace LogId {
enum Type: std::uint16_t
{
	MAIN_LOG = 1
};
} // namespace LogId

namespace ModuleType {

enum Type: std::uint16_t
{
	INVALID = 0,
	LIGHTS_FILE = 1,
	LIGHTS_LOGGER = 2,
	LIGHTS_TEST = 3,
	MAX,
};

const char* str[MAX] = {};

void init_str()
{
#define LIGHTS_EXAMPLE_DEFINE_STR(symbol) str[symbol] = #symbol;
	LIGHTS_EXAMPLE_DEFINE_STR(LIGHTS_FILE);
	LIGHTS_EXAMPLE_DEFINE_STR(LIGHTS_LOGGER);
	LIGHTS_EXAMPLE_DEFINE_STR(LIGHTS_TEST);
}

} // namespace ModuleType


void example_TextLogger()
{
	auto stdout_sink = lights::log_sinks::StdoutSink::instance();
	lights::TextLogger<lights::log_sinks::StdoutSink> logger("test", stdout_sink);

	// Set which level can be log.
	logger.set_level(lights::LogLevel::DEBUG);

	ModuleType::init_str();

	// If want to record module.
	logger.set_module_name_handler([](std::uint16_t id) {
		return ModuleType::str[id];
	});

	try
	{
		lights::FileStream file("unexists_file", "r");
	}
	catch (const lights::Exception& ex)
	{
		// This macro will record the source location.
		LIGHTS_ERROR(logger, ModuleType::LIGHTS_FILE, ex);
	}

	// Can use format library in-place.
	LIGHTS_DEBUG(logger, ModuleType::LIGHTS_TEST, "Current timestamp is {}", lights::current_timestamp());
	LIGHTS_INFO(logger, ModuleType::LIGHTS_TEST, "Only for a test");

	// Rotate everyday.
	auto time_file_sink = std::make_shared<lights::log_sinks::TimeRotatingFileSink>("daily_logger.log");
	lights::TextLogger<lights::log_sinks::TimeRotatingFileSink> daily_logger("daily_logger", time_file_sink);
	LIGHTS_INFO(logger, ModuleType::LIGHTS_TEST, "Only for a test");
}


void example_BinaryLogger()
{
	auto str_table = lights::StringTable::create("log_str_table");
	lights::StringView log_filename = "example_log.log";
	auto file_sink = std::make_shared<lights::log_sinks::SimpleFileSink>(log_filename);
	lights::BinaryLogger<lights::log_sinks::SimpleFileSink> logger(LogId::MAIN_LOG, file_sink, str_table);

	logger.set_level(lights::LogLevel::DEBUG);

	// Turn off only ModuleType::LIGHTS_TEST module log.
	std::vector<lights::LogLevel> module_log_level(ModuleType::MAX, lights::LogLevel::DEBUG);
	module_log_level[ModuleType::LIGHTS_TEST] = lights::LogLevel::OFF;

	// Set which level and module can be log.
	logger.set_module_should_log_handler(
		[&module_log_level](lights::LogLevel level, std::uint16_t module_id) {
			return level >= module_log_level[module_id];
		});

	try
	{
		lights::FileStream file("unexists_file", "r");
	}
	catch (const lights::Exception& ex)
	{
		LIGHTS_ERROR(logger, ModuleType::LIGHTS_FILE, ex);
	}

	LIGHTS_DEBUG(logger, ModuleType::LIGHTS_TEST, "Current timestamp is {}", lights::current_timestamp());
	LIGHTS_INFO(logger, ModuleType::LIGHTS_TEST, "Only for a test");

	// Read binary log.
	lights::BinaryLogReader reader(log_filename, str_table);
	while (!reader.eof())
	{
		auto log = reader.read();
		if (log.data() == nullptr)
		{
			break;
		}
		lights::stdout_stream().write_line(log);
	}
}


void example_log()
{
	example_TextLogger();
	example_BinaryLogger();
}

} // namespace example
} // namespace lights
