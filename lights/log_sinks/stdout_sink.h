/**
 * stdout_sink.h
 * @author wherewindblow
 * @date   Dec 18, 2016
 */

#pragma once

#include <cstddef>
#include <cstdio>
#include <memory>

#include "../sequence.h"
#include "../file.h"

namespace lights {
namespace log_sinks {

/**
 * StdoutSink is wrapper sink of stdout_stream.
 */
class StdoutSink: public SinkAdapter
{
public:
	/**
	 * Writes log_msg to stdout_stream.
	 */
	std::size_t write(SequenceView log_msg) override
	{
		return stdout_stream().write(log_msg);
	}

	/**
	 * Returns instance.
	 */
	static std::shared_ptr<StdoutSink> instance()
	{
		static auto instance = std::make_shared<StdoutSink>();
		return instance;
	};
};


/**
 * StderrSink is wrapper sink of stderr_stream.
 */
class StderrSink: public SinkAdapter
{
public:
	/**
	 * Writes log_msg to stderr_stream.
	 */
	std::size_t write(SequenceView log_msg) override
	{
		return stderr_stream().write(log_msg);
	}

	/**
	 * Returns instance.
	 */
	static std::shared_ptr<StderrSink> instance()
	{
		static auto instance = std::make_shared<StderrSink>();
		return instance;
	};
};

} // namespace log_sinks
} // namespace lights
