/**
 * logger.h
 * @author wherewindblow
 * @date   Dec 09, 2016
 */

#pragma once

#include <cstring>
#include <ctime>
#include <string>

#include "env.h"
#include "format.h"
#include "format/binary_format.h"
#include "file.h"
#include "exception.h"
#include "string_table.h"


namespace lights {

namespace details {

static const StringView log_level_names[] = {
	"debug", "info", "warnning", "error", "off"
};

} // namespace details


/**
 * Log message level.
 */
enum class LogLevel: std::uint8_t
{
	DEBUG = 0, INFO, WARN, ERROR, OFF
};

/**
 * Converts log level to string.
 */
inline const StringView to_string(LogLevel level)
{
	return details::log_level_names[static_cast<std::uint8_t>(level)];
}


/**
 * PreciseTime use to record hight resolution time point.
 */
struct PreciseTime
{
	PreciseTime() = default;

	PreciseTime(std::int64_t seconds, std::int64_t nanoseconds):
		seconds(seconds), nanoseconds(nanoseconds)
	{}

	std::int64_t seconds;
	std::int64_t nanoseconds;
};


inline bool is_over_flow(std::int64_t a, std::int64_t b)
{
	std::int64_t x = a + b;
	return ((x^a) < 0 && (x^b) < 0);
}

inline PreciseTime operator+(const PreciseTime& left, const PreciseTime& right)
{
	PreciseTime result(left.seconds + right.seconds, left.nanoseconds + right.nanoseconds);
	if (is_over_flow(left.nanoseconds, right.nanoseconds))
	{
		++result.seconds;
	}
	return result;
}

inline PreciseTime operator-(const PreciseTime& left, const PreciseTime& right)
{
	PreciseTime result(left.seconds - right.seconds, left.nanoseconds - right.nanoseconds);
	if (left.nanoseconds < right.nanoseconds)
	{
		--result.seconds;
		result.nanoseconds = std::abs(result.nanoseconds);
	}
	return result;
}

inline bool operator<(const PreciseTime& left, const PreciseTime& right)
{
	if (left.seconds < right.seconds)
	{
		return true;
	}
	return left.nanoseconds < right.nanoseconds;
}

inline bool operator>(const PreciseTime& left, const PreciseTime& right)
{
	return !(left < right);
}

/**
 * Returns the current time point.
 */
PreciseTime current_precise_time();

/**
 * Puts precise time to format sink.
 */
template <typename Backend>
inline void to_string(FormatSink<Backend> sink, const PreciseTime& time)
{
	sink << time.seconds << '.' << time.nanoseconds << 's';
}


/**
 * General log sink pointer.
 */
using LogSinkPtr = std::shared_ptr<Sink>;


/**
 * TextLogger log message with text mode to backend sink.
 */
class TextLogger
{
public:
	/**
	 * Creates text logger.
	 */
	TextLogger(StringView name, LogSinkPtr sink_ptr);

	/**
	 * Returns logger name.
	 */
	const std::string& get_name() const
	{
		return m_name;
	}

	/**
	 * Returns logger level.
	 */
	LogLevel get_level() const
	{
		return m_level;
	}

	/**
	 * Sets logger level and all log message level is greater or equal to this
	 * level will be record to sink.
	 */
	void set_level(LogLevel level)
	{
		m_level = level;
	}

	/**
	 * Checks is open switch of record source location. The default value is open.
	 */
	bool is_record_location() const
	{
		return m_record_location;
	}

	/**
	 * Sets switch of record source location.
	 * @note Open switch can get more info, but also will raise output.
	 */
	void set_record_location(bool enable_record)
	{
		m_record_location = enable_record;
	}

	/**
	 * Formats @c fmt with @ args and log to sink.
	 * @param level     Level of log message.
	 * @param location  Where call this function.
	 * @param fmt       Format string of log message.
	 * @param args      Arguments of format.
	 */
	template <typename ... Args>
	void log(LogLevel level, const SourceLocation& location, const char* fmt, const Args& ... args);

	/**
	 * Logs str to sink.
	 * @param level     Level of log message.
	 * @param location  Where call this function.
	 * @param str       Log message.
	 */
	void log(LogLevel level, const SourceLocation& location, const char* str);

	/**
	 * Logs value to sink.
	 * @param level     Level of log message.
	 * @param location  Where call this function.
	 * @param value     Any type that can be format.
	 */
	template <typename T>
	void log(LogLevel level, const SourceLocation& location, const T& value);

private:
	bool should_log(LogLevel level) const
	{
		return m_level <= level;
	}

	void generate_signature(LogLevel level);

	void recore_location(const SourceLocation& location)
	{
		if (is_record_location() && is_valid(location))
		{
			m_writer.write(" [{}:{}][{}]", location.file(), location.line(), location.function());
		}
	}

	void append_log_seperator();

	std::string m_name;
	LogLevel m_level = LogLevel::INFO;
	bool m_record_location = true;
	LogSinkPtr m_sink_ptr;
	char m_write_target[WRITER_BUFFER_SIZE_DEFAULT];
	TextWriter m_writer;
};


template <typename ... Args>
void TextLogger::log(LogLevel level, const SourceLocation& location, const char* fmt, const Args& ... args)
{
	if (this->should_log(level))
	{
		m_writer.clear();
		this->generate_signature(level);
		m_writer.write(fmt, args ...);
		this->recore_location(location);
		append_log_seperator();
		m_sink_ptr->write(m_writer.string_view());
	}
}


template <typename T>
void TextLogger::log(LogLevel level, const SourceLocation& location, const T& value)
{
	if (this->should_log(level))
	{
		m_writer.clear();
		this->generate_signature(level);
		m_writer << value;
		this->recore_location(location);
		append_log_seperator();
		m_sink_ptr->write(m_writer.string_view());
	}
}


/**
 * BinaryMessageSignature is binary log message common header.
 */
struct BinaryMessageSignature
{
public:
	std::int64_t time_seconds;
	std::int64_t time_nanoseconds;
	std::uint32_t file_id;
	std::uint32_t function_id;
	std::uint32_t source_line;
	std::uint32_t description_id;
	std::uint16_t logger_id;
	std::uint16_t argument_length;
	LogLevel level;
} LIGHTS_NOT_MEMEORY_ALIGNMENT;


/**
 * BinaryLogger logs message with binary mode to the backend sink. Binary log message is
 * optimized with output, so can save output and record more information. On the other hand,
 * binary log message is structured and can be convenient analyse.
 * Binary log message can use BinaryLogReader to read it.
 */
class BinaryLogger
{
public:
	/**
	 * Creates binary logger.
	 */
	BinaryLogger(std::uint16_t logger_id, LogSinkPtr sink_ptr, StringTablePtr str_table_ptr);

	/**
	 * Gets logger id.
	 */
	std::uint16_t get_logger_id() const
	{
		return m_signature->logger_id;
	}

	/**
	 * Gets logger level.
	 */
	LogLevel get_level() const
	{
		return m_level;
	}

	/**
	 * Sets the logger level.
	 */
	void set_level(LogLevel level)
	{
		m_level = level;
	}

	/**
	 * Formats @c fmt with @ args and log to sink.
	 * @param level     Level of log message.
	 * @param location  Where call this function.
	 * @param fmt       Format string of log message.
	 * @param args      Arguments of format.
	 */
	template <typename ... Args>
	void log(LogLevel level, const SourceLocation& location, const char* fmt, const Args& ... args);

	/**
	 * Logs str to sink.
	 * @param level     Level of log message.
	 * @param location  Where call this function.
	 * @param str       Log message.
	 */
	void log(LogLevel level, const SourceLocation& location, const char* str);

	/**
	 * Logs value to sink.
	 * @param level     Level of log message.
	 * @param location  Where call this function.
	 * @param value     Any type that can be format.
	 */
	template <typename T>
	void log(LogLevel level, const SourceLocation& location, const T& value);

private:
	bool should_log(LogLevel level) const
	{
		return m_level <= level;
	}

	void generate_signature(LogLevel level, const SourceLocation& location, StringView description);

	void set_argument_length(std::uint16_t length)
	{
		m_signature->argument_length = length;
		char* tail_length = m_write_target + sizeof(BinaryMessageSignature) + m_writer.length();
		*reinterpret_cast<std::uint16_t*>(tail_length) = m_signature->argument_length;
	}

	void sink_msg()
	{
		// signature + content + tail length.
		SequenceView view(m_write_target, sizeof(BinaryMessageSignature) + m_writer.size() + sizeof(std::uint16_t));
		m_sink_ptr->write(view);
	}

	LogLevel m_level = LogLevel::INFO;
	LogSinkPtr m_sink_ptr;
	StringTablePtr m_str_table_ptr;
	char m_write_target[WRITER_BUFFER_SIZE_LARGE];
	BinaryMessageSignature* m_signature;
	BinaryStoreWriter m_writer;
};


template <typename ... Args>
void BinaryLogger::log(LogLevel level, const SourceLocation& location, const char* fmt, const Args& ... args)
{
	if (this->should_log(level))
	{
		this->generate_signature(level, location, fmt);

		m_writer.clear();
		m_writer.write(fmt, args ...);
		this->set_argument_length(static_cast<std::uint16_t>(m_writer.length()));
		this->sink_msg();
	}
}


template <typename T>
void BinaryLogger::log(LogLevel level, const SourceLocation& location, const T& value)
{
	if (this->should_log(level))
	{
		const StringView description = "{}";
		this->generate_signature(level, location, description);

		m_writer.clear();
		m_writer.write(description, value);
		this->set_argument_length(static_cast<std::uint16_t>(m_writer.length()));
		this->sink_msg();
	}
}


/**
 * BinaryLogReader can read the log file that write by BinaryLogger.
 */
class BinaryLogReader
{
public:
	BinaryLogReader(StringView log_filename, StringTablePtr str_table_ptr);

	/**
	 * @note Returns nullptr when have no log message to read.
	 */
	StringView read();

	/**
	 * Jumps to the specify line.
	 * @param line  When line is positive will jump to the line start from head.
	 *              When line is negative will jump to the line start from tail.
	 */
	void jump(std::streamoff line);

	/**
	 * Jumps to file end.
	 */
	void jump_to_end()
	{
		m_file.seek(0, FileSeekWhence::END);
	}

	/**
	 * Returns is end of file.
	 */
	bool eof()
	{
		m_file.peek();
		return m_file.eof();
	}

	/**
	 * Checks have new message.
	 */
	bool have_new_message()
	{
		return static_cast<std::size_t>(m_file.tell()) < m_file.size();
	}

	/**
	 * Clear end of file flag.
	 */
	void clear_eof()
	{
		m_file.clear_error();
	}

private:
	void jump_from_head(std::size_t line);

	void jump_from_tail(std::size_t line);

	FileStream m_file;
	StringTablePtr m_str_table_ptr;
	BinaryMessageSignature m_signature;
	char m_write_target[WRITER_BUFFER_SIZE_LARGE];
	BinaryRestoreWriter m_writer;
};


#ifdef LIGHTS_OPEN_LOG
#	define LIGHTS_LOG(logger, level, ...) \
		logger.log(level, LIGHTS_CURRENT_SOURCE_LOCATION, __VA_ARGS__);
#else
#	define LIGHTS_LOG(logger, level, ...)
#endif


/**
 * Unified interface of logger to log message.
 * @param ... Can use format string and arguments or just a any type value.
 */
#define LIGHTS_DEBUG(logger, ...) \
	LIGHTS_LOG(logger, lights::LogLevel::DEBUG, __VA_ARGS__);
#define LIGHTS_INFO(logger, ...) \
	LIGHTS_LOG(logger, lights::LogLevel::INFO, __VA_ARGS__);
#define LIGHTS_WARN(logger, ...) \
	LIGHTS_LOG(logger, lights::LogLevel::WARN, __VA_ARGS__);
#define LIGHTS_ERROR(logger, ...) \
	LIGHTS_LOG(logger, lights::LogLevel::ERROR, __VA_ARGS__);

} // namespace lights
