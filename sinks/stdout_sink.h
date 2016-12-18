/**
 * stdout_sink.h
 * @author wherewindblow
 * @date   Dec 18, 2016
 */

#pragma once

#include <cstddef>
#include <cstdio>
#include <memory>


namespace lights {
namespace sinks {

class StdoutSink
{
public:
	void write(const char* str, std::size_t len)
	{
		std::fwrite(str, sizeof(char), len, stdout);
	}

	static std::shared_ptr<StdoutSink> get_instance()
	{
		static auto instance = std::make_shared<StdoutSink>();
		return instance;
	};
};


class StderrSink
{
public:
	void write(const char* str, std::size_t len)
	{
		std::fwrite(str, sizeof(char), len, stderr);
	}

	static std::shared_ptr<StderrSink> get_instance()
	{
		static auto instance = std::make_shared<StderrSink>();
		return instance;
	};
};

} // namespace sinks
} // namespace lights
