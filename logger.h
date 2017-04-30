/**
 * logger.h
 * @author wherewindblow
 * @date   Dec 09, 2016
 */

#pragma once

#include <cstring>
#include <ctime>
#include <memory>

#include <time.h>

#include "format.h"


namespace lights {

namespace details {

static const std::string log_level_names[] = {
	"DEBUG", "INFO", "WARNNING", "ERROR", "OFF"
};

} // namespace details


enum class LogLevel
{
	DEBUG = 0, INFO, WARN, ERROR, OFF
};


inline const std::string& to_string(LogLevel level)
{
	return details::log_level_names[static_cast<int>(level)];
}


/**
 * Logger message to the backend sink.
 * @tparam Sink  Support `void write(const char* str, std::size_t len);`
 *
 * Message is compose by a signature header and message body.
 * Signature is compose by time, logger name and log level.
 */
template <typename Sink>
class Logger
{
public:
	Logger(const std::string& name, std::shared_ptr<Sink> sink);

	const std::string& get_name() const
	{
		return m_name;
	}

	LogLevel get_level() const
	{
		return m_level;
	}

	void set_level(LogLevel level)
	{
		m_level = level;
	}


	template <typename ... Args>
	void log(LogLevel level, const char* fmt, const Args& ... args);

	template <typename ... Args>
	void debug(const char* fmt, const Args& ... args)
	{
		this->log(LogLevel::DEBUG, fmt, args ...);
	}

	template <typename ... Args>
	void info(const char* fmt, const Args& ... args)
	{
		this->log(LogLevel::INFO, fmt, args ...);
	}

	template <typename ... Args>
	void warn(const char* fmt, const Args& ... args)
	{
		this->log(LogLevel::WARN, fmt, args ...);
	}

	template <typename ... Args>
	void error(const char* fmt, const Args& ... args)
	{
		this->log(LogLevel::ERROR, fmt, args ...);
	}


	void log(LogLevel level, const char* str);

	void debug(const char* str)
	{
		this->log(LogLevel::DEBUG, str);
	}

	void info(const char* str)
	{
		this->log(LogLevel::INFO, str);
	}

	void warn(const char* str)
	{
		this->log(LogLevel::WARN, str);
	}

	void error(const char* str)
	{
		this->log(LogLevel::ERROR, str);
	}


	template <typename T>
	void log(LogLevel level, const T& value);

	template <typename T>
	void debug(const T& value)
	{
		this->log(LogLevel::DEBUG, value);
	}

	template <typename T>
	void info(const T& value)
	{
		this->log(LogLevel::INFO, value);
	}

	template <typename T>
	void warn(const T& value)
	{
		this->log(LogLevel::WARN, value);
	}

	template <typename T>
	void error(const T& value)
	{
		this->log(LogLevel::ERROR, value);
	}

private:
	bool should_log(LogLevel level) const
	{
		return m_level <= level;
	}

	std::string get_signature_header();

	std::string m_name;
	LogLevel m_level = LogLevel::INFO;
	std::shared_ptr<Sink> m_sink;
	MemoryWriter<500> m_writer;
};


template <typename Sink>
Logger<Sink>::Logger(const std::string& name, std::shared_ptr<Sink> sink) :
	m_name(name), m_sink(sink) {}


template <typename Sink>
template <typename ... Args>
void Logger<Sink>::log(LogLevel level, const char* fmt, const Args& ... args)
{
	if (this->should_log(level))
	{
		std::string msg = this->get_signature_header();
		write(make_string_adapter(msg), fmt, args ...);
		msg.push_back('\n');
		m_sink->write(msg.c_str(), msg.length());
	}
}


template <typename Sink>
void Logger<Sink>::log(LogLevel level, const char* str)
{
	if (this->should_log(level))
	{
		std::string msg = this->get_signature_header();
		msg.append(str);
		msg.push_back('\n');
		m_sink->write(msg.c_str(), msg.length());
	}
}


template <typename Sink>
template <typename T>
void Logger<Sink>::log(LogLevel level, const T& value)
{
	if (this->should_log(level))
	{
		std::string msg = this->get_signature_header();
		append(make_string_adapter(msg), value);
		msg.push_back('\n');
		m_sink->write(msg.c_str(), msg.length());
	}
}


template <typename Sink>
std::string Logger<Sink>::get_signature_header()
{
	std::time_t time = std::time(nullptr);
	std::tm tm;
	localtime_r(&time, &tm);
	char buf[50];
	std::size_t len = std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
	buf[len] = '\0';
	std::string header;
	header.reserve(200);
	auto str = make_string_adapter(header);
	write(str, "[{}] [{}] [{}] ", StringView{ buf, len }, m_name, to_string(m_level));
	return header;
}

} // namespace lights
