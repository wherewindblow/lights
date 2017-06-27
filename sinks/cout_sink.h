/**
 * cout_sink.h
 * @author wherewindblow
 * @date   Dec 18, 2016
 */

#pragma once

#include <cstddef>
#include <iostream>
#include <memory>


namespace lights {
namespace sinks {

class CoutSink
{
public:
	void write(const void* buf, std::size_t len)
	{
		std::cout.write(reinterpret_cast<const char*>(buf), len);
	}

	static std::shared_ptr<CoutSink> instance()
	{
		static auto instance = std::make_shared<CoutSink>();
		return instance;
	};
};


class CerrSink
{
public:
	void write(const void* buf, std::size_t len)
	{
		std::cerr.write(reinterpret_cast<const char*>(buf), len);
	}

	static std::shared_ptr<CerrSink> instance()
	{
		static auto instance = std::make_shared<CerrSink>();
		return instance;
	};
};


class ClogSink
{
public:
	void write(const char* str, std::size_t len)
	{
		std::clog.write(str, len);
	}

	static std::shared_ptr<ClogSink> instance()
	{
		static auto instance = std::make_shared<ClogSink>();
		return instance;
	};
};

} // namespace sinks
} // namespace lights