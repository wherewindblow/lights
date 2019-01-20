/**
 * cout_sink.h
 * @author wherewindblow
 * @date   Dec 18, 2016
 */

#pragma once

#include <cstddef>
#include <iostream>

#include "../sequence.h"
#include "../ostream.h"


namespace lights {
namespace log_sinks {

/**
 * CoutSink is wrapper sink of std::cout.
 */
class CoutSink: public Sink
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
	static CoutSink& instance()
	{
		static CoutSink inst;
		return inst;
	};
};


/**
 * CoutSink is wrapper sink of std::cerr.
 */
class CerrSink: public Sink
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
	static CerrSink& instance()
	{
		static CerrSink inst;
		return inst;
	};
};


/**
 * CoutSink is wrapper sink of std::clog.
 */
class ClogSink: public Sink
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
	static ClogSink& instance()
	{
		static ClogSink inst;
		return inst;
	};
};

} // namespace log_sinks
} // namespace lights