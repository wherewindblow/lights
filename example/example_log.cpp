/**
 * example_log.cpp
 * @author wherewindblow
 * @date   Aug 23, 2017
 */

#include "example_log.h"

#include <vector>

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


void example_TextLogger()
{
	auto stdout_sink = lights::log_sinks::StdoutSink::instance();
	lights::TextLogger logger("test", stdout_sink);

	// Set which level can be log.
	logger.set_level(lights::LogLevel::DEBUG);

	try
	{
		lights::FileStream file("unexists_file", "r");
	}
	catch (const lights::Exception& ex)
	{
		// This macro will record the source location.
		LIGHTS_ERROR(logger, ex);
	}

	// Can use format library in-place.
	LIGHTS_DEBUG(logger, "Current timestamp is {}", lights::current_timestamp());
	LIGHTS_INFO(logger, "Only for a test");

	// Rotate everyday.
	auto time_file_sink = std::make_shared<lights::log_sinks::TimeRotatingFileSink>("daily_logger.log");
	lights::TextLogger daily_logger("daily_logger", time_file_sink);
	LIGHTS_INFO(logger, "Only for a test");
}


void example_BinaryLogger()
{
	auto str_table_ptr = lights::StringTable::create("log_str_table");
	lights::StringView log_filename = "example_log.log";
	auto file_sink = std::make_shared<lights::log_sinks::SimpleFileSink>(log_filename);
	lights::BinaryLogger logger(LogId::MAIN_LOG, file_sink, str_table_ptr);

	logger.set_level(lights::LogLevel::DEBUG);

	try
	{
		lights::FileStream file("unexists_file", "r");
	}
	catch (const lights::Exception& ex)
	{
		LIGHTS_ERROR(logger, ex);
	}

	LIGHTS_DEBUG(logger, "Current timestamp is {}", lights::current_timestamp());
	LIGHTS_INFO(logger, "Only for a test");

	// Read binary log.
	lights::BinaryLogReader reader(log_filename, str_table_ptr);
	while (!reader.eof())
	{
		auto log = reader.read();
		if (!is_valid(log))
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
