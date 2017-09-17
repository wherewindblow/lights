/**
 * cout_sink.h
 * @author wherewindblow
 * @date   Dec 18, 2016
 */

#pragma once

#include <cstddef>
#include <iostream>
#include <memory>

#include "../sequence.h"
#include "../ostream.h"


namespace lights {
namespace log_sinks {

class CoutSink: public SinkAdapter
{
public:
	std::size_t write(SequenceView sequence) override
	{
		std::cout << sequence;
		return sequence.length();
	}

	static std::shared_ptr<CoutSink> instance()
	{
		static auto instance = std::make_shared<CoutSink>();
		return instance;
	};
};


class CerrSink: public SinkAdapter
{
public:
	std::size_t write(SequenceView sequence) override
	{
		std::cerr << sequence;
		return sequence.length();
	}

	static std::shared_ptr<CerrSink> instance()
	{
		static auto instance = std::make_shared<CerrSink>();
		return instance;
	};
};


class ClogSink: public SinkAdapter
{
public:
	std::size_t  write(SequenceView sequence) override
	{
		std::clog << sequence;
		return sequence.length();
	}

	static std::shared_ptr<ClogSink> instance()
	{
		static auto instance = std::make_shared<ClogSink>();
		return instance;
	};
};

} // namespace log_sinks
} // namespace lights