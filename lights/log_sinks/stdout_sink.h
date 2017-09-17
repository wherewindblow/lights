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

class StdoutSink: public SinkAdapter
{
public:
	std::size_t write(SequenceView sequence) override
	{
		return stdout_stream().write(sequence);
	}

	static std::shared_ptr<StdoutSink> instance()
	{
		static auto instance = std::make_shared<StdoutSink>();
		return instance;
	};
};


class StderrSink: public SinkAdapter
{
public:
	std::size_t write(SequenceView sequence) override
	{
		return stderr_stream().write(sequence);
	}

	static std::shared_ptr<StderrSink> instance()
	{
		static auto instance = std::make_shared<StderrSink>();
		return instance;
	};
};

} // namespace log_sinks
} // namespace lights
