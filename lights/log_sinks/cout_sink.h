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

/**
 * CoutSink is wrapper sink of std::cout.
 */
class CoutSink: public SinkAdapter
{
public:
	/**
	 * Writes log_msg to std::cout.
	 */
	std::size_t write(SequenceView log_msg) override
	{
		std::cout << log_msg;
		return log_msg.length();
	}

	/**
	 * Returns instance.
	 */
	static std::shared_ptr<CoutSink> instance()
	{
		static auto instance = std::make_shared<CoutSink>();
		return instance;
	};
};


/**
 * CoutSink is wrapper sink of std::cerr.
 */
class CerrSink: public SinkAdapter
{
public:
	/**
	 * Writes log_msg to std::cerr.
	 */
	std::size_t write(SequenceView log_msg) override
	{
		std::cerr << log_msg;
		return log_msg.length();
	}

	/**
	 * Returns instance.
	 */
	static std::shared_ptr<CerrSink> instance()
	{
		static auto instance = std::make_shared<CerrSink>();
		return instance;
	};
};


/**
 * CoutSink is wrapper sink of std::clog.
 */
class ClogSink: public SinkAdapter
{
public:
	/**
	 * Writes log_msg to std::clog.
	 */
	std::size_t write(SequenceView log_msg) override
	{
		std::clog << log_msg;
		return log_msg.length();
	}

	/**
	 * Returns instance.
	 */
	static std::shared_ptr<ClogSink> instance()
	{
		static auto instance = std::make_shared<ClogSink>();
		return instance;
	};
};

} // namespace log_sinks
} // namespace lights