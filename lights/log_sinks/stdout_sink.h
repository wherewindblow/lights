/**
 * stdout_sink.h
 * @author wherewindblow
 * @date   Dec 18, 2016
 */

#pragma once

#include <cstddef>
#include <cstdio>
#include <memory>

#include "lights/block_description.h"
#include "../file.h"

namespace lights {
namespace log_sinks {

class StdoutSink
{
public:
	void write(BufferView buffer)
	{
		stdout_stream().write(buffer);
	}

	static std::shared_ptr<StdoutSink> instance()
	{
		static auto instance = std::make_shared<StdoutSink>();
		return instance;
	};
};


class StderrSink
{
public:
	void write(BufferView buffer)
	{
		stderr_stream().write(buffer);
	}

	static std::shared_ptr<StderrSink> instance()
	{
		static auto instance = std::make_shared<StderrSink>();
		return instance;
	};
};

} // namespace log_sinks
} // namespace lights
