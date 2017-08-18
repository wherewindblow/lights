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

class CoutSink
{
public:
	void write(SequenceView sequence)
	{
		std::cout << sequence;
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
	void write(SequenceView sequence)
	{
		std::cerr << sequence;
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
	void write(SequenceView sequence)
	{
		std::clog << sequence;
	}

	static std::shared_ptr<ClogSink> instance()
	{
		static auto instance = std::make_shared<ClogSink>();
		return instance;
	};
};

} // namespace log_sinks
} // namespace lights