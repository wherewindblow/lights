/**
 * logger.h
 * @author wherewindblow
 * @date   Dec 09, 2016
 */

#pragma once

#include <cstring>
#include <ctime>
#include <memory>
#include <chrono>

#include <time.h>

#include "format.h"


namespace lights {

namespace details {

static const std::string log_level_names[] = {
	"debug", "info", "warnning", "error", "off"
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

	template <std::size_t N>
	static lights::MemoryWriter<N>&  write_2_digit(lights::MemoryWriter<N>& writer, unsigned num)
	{
		if (num >= 10)
		{
			writer << num;
		}
		else
		{
			writer << '0' << num;
		}
		return writer;
	}

	void generate_signature_header();

	std::string m_name;
	LogLevel m_level = LogLevel::INFO;
	std::shared_ptr<Sink> m_sink;
	MemoryWriter<> m_writer;
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
		m_writer.clear();
		this->generate_signature_header();
		m_writer.write(fmt, args ...);
		m_writer.append('\n');
		lights::StringView view = m_writer.str_view();
		m_sink->write(view.string, view.length);
	}
}


template <typename Sink>
void Logger<Sink>::log(LogLevel level, const char* str)
{
	if (this->should_log(level))
	{
		m_writer.clear();
		this->generate_signature_header();
		m_writer << str << '\n';
		lights::StringView view = m_writer.str_view();
		m_sink->write(view.string, view.length);
	}
}


template <typename Sink>
template <typename T>
void Logger<Sink>::log(LogLevel level, const T& value)
{
	if (this->should_log(level))
	{
		m_writer.clear();
		this->generate_signature_header();
		m_writer << value << '\n';
		lights::StringView view = m_writer.str_view();
		m_sink->write(view.string, view.length);
	}
}

/**
 * Use second as a tick to record timestamp.
 * It's faster than use chrono, but precision is not enought.
 */
//template <typename Sink>
//void Logger<Sink>::generate_signature_header()
//{
//	std::time_t time = std::time(nullptr);
//	std::tm tm;
//	localtime_r(&time, &tm);
//
//	m_writer << '[';
//
//	m_writer << static_cast<unsigned>(tm.tm_year + 1900) << '-';
//	write_2_digit(m_writer, static_cast<unsigned>(tm.tm_mon + 1)) << '-';
//	write_2_digit(m_writer, static_cast<unsigned>(tm.tm_mday)) << ' ';
//	write_2_digit(m_writer, static_cast<unsigned>(tm.tm_hour)) << ':';
//	write_2_digit(m_writer, static_cast<unsigned>(tm.tm_min)) << ':';
//	write_2_digit(m_writer, static_cast<unsigned>(tm.tm_sec));
//
//	m_writer << "] [" << m_name << "] [" << to_string(m_level) << "] ";
//}

template <typename Sink>
void Logger<Sink>::generate_signature_header()
{
	namespace chrono = std::chrono;
	auto chrono_time = chrono::system_clock::now();
	std::time_t time = chrono::system_clock::to_time_t(chrono_time);
	std::tm tm;
	localtime_r(&time, &tm);

	m_writer << '[';

	m_writer << static_cast<unsigned>(tm.tm_year + 1900) << '-';
	write_2_digit(m_writer, static_cast<unsigned>(tm.tm_mon + 1)) << '-';
	write_2_digit(m_writer, static_cast<unsigned>(tm.tm_mday)) << ' ';
	write_2_digit(m_writer, static_cast<unsigned>(tm.tm_hour)) << ':';
	write_2_digit(m_writer, static_cast<unsigned>(tm.tm_min)) << ':';
	write_2_digit(m_writer, static_cast<unsigned>(tm.tm_sec)) << '.';

	auto duration = chrono_time.time_since_epoch();
	auto millis = chrono::duration_cast<chrono::milliseconds>(duration).count() % 1000;
	m_writer << pad(static_cast<unsigned>(millis), '0', 3);

	m_writer << "] [" << m_name << "] [" << to_string(m_level) << "] ";
}

} // namespace lights
