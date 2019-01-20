/**
 * example_log.cpp
 * @author wherewindblow
 * @date   Aug 23, 2017
 */

#include "example_log.h"

#include <vector>

#include <lights/logger.h>
#include <lights/sinks/file_sink.h>
#include <lights/sinks/stdout_sink.h>


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
	lights::Sink& stdout_sink = lights::sinks::StdoutSink::instance();
	lights::TextLogger logger("test", stdout_sink);

	// Set which level can be log.
	logger.set_level(lights::LogLevel::DEBUG);

	try
	{
		lights::FileStream file("not_exists_file", "r");
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
	lights::sinks::TimeRotatingFileSink time_file_sink("daily_logger.log");
	lights::TextLogger daily_logger("daily_logger", time_file_sink);
	LIGHTS_INFO(logger, "Only for a test");
}


void example_BinaryLogger()
{
	lights::StringView log_filename = "example_log.log";
	lights::sinks::SimpleFileSink file_sink(log_filename);
	lights::StringTable str_table("log_str_table");
	lights::BinaryLogger logger("bin_log", file_sink, str_table);

	logger.set_level(lights::LogLevel::DEBUG);

	try
	{
		lights::FileStream file("not_exists_file", "r");
	}
	catch (const lights::Exception& ex)
	{
		LIGHTS_ERROR(logger, ex);
	}

	LIGHTS_DEBUG(logger, "Current timestamp is {}", lights::current_timestamp());
	LIGHTS_INFO(logger, "Only for a test");

	// Read binary log.
	lights::BinaryLogReader reader(log_filename, str_table);
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
