/**
 * stdout_sink.h
 * @author wherewindblow
 * @date   Dec 18, 2016
 */

#pragma once

#include <cstddef>
#include <cstdio>

#include "../sequence.h"
#include "../file.h"


namespace lights {
namespace sinks {

/**
 * StdoutSink is wrapper sink of stdout_stream.
 */
class StdoutSink: public Sink
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
	static StdoutSink& instance()
	{
		static StdoutSink inst;
		return inst;
	};
};


/**
 * StderrSink is wrapper sink of stderr_stream.
 */
class StderrSink: public Sink
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
	static StderrSink& instance()
	{
		static StderrSink inst;
		return inst;
	};
};

} // namespace sinks
} // namespace lights
