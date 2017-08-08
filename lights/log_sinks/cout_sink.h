/**
 * cout_sink.h
 * @author wherewindblow
 * @date   Dec 18, 2016
 */

#pragma once

#include <cstddef>
#include <iostream>
#include <memory>

#include "lights/block_description.h"


namespace lights {
namespace log_sinks {

class CoutSink
{
public:
	void write(BufferView buffer)
	{
		std::cout << buffer;
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
	void write(BufferView buffer)
	{
		std::cerr << buffer;
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
	void write(BufferView buffer)
	{
		std::clog << buffer;
	}

	static std::shared_ptr<ClogSink> instance()
	{
		static auto instance = std::make_shared<ClogSink>();
		return instance;
	};
};

} // namespace log_sinks
} // namespace lights