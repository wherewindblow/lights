/**
 * logger.h
 * @author wherewindblow
 * @date   Dec 09, 2016
 */

#pragma once

#include <cstring>
#include <ctime>

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


enum class LogLevel: std::uint8_t
{
	DEBUG = 0, INFO, WARN, ERROR, OFF
};


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

/**
 * Returns the current time point.
 */
PreciseTime get_precise_time();


/**
 * Logs message with readable text to the backend sink.
 *
 * Message is compose by a signature header and message body.
 * Signature is compose by time, logger name, log level, log module and source location.
 */

using LogSinkPtr = std::shared_ptr<SinkAdapter>;

class TextLogger
{
public:
	using ModuleNameHandler = std::function<StringView(std::uint16_t)>;
	using ModuleShouldLogHandler = std::function<bool(LogLevel, std::uint16_t)>;

	TextLogger(StringView name, LogSinkPtr sink_ptr);

	/**
	 * Returns the name of this logger.
	 */
	const std::string& get_name() const
	{
		return m_name;
	}

	/**
	 * Returns the level of this logger.
	 */
	LogLevel get_level() const
	{
		return m_level;
	}

	/**
	 * Sets the level of this logger and all log message level is greater or equal to this
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
	 * Checks is open switch of record module. The default value is open.
	 */
	bool is_record_module() const
	{
		return m_record_module;
	}

	void set_record_module(bool enable_record)
	{
		m_record_module = enable_record;
	}

	ModuleNameHandler get_module_name_handler() const
	{
		return m_module_name_handler;
	}

	/**
	 * Sets the handler to convert module id to name.
	 */
	void set_module_name_handler(ModuleNameHandler module_name_handler)
	{
		m_module_name_handler = module_name_handler;
	}

	ModuleShouldLogHandler get_module_should_log_handler() const
	{
		return m_module_should_log;
	}

	/**
	 * Sets the module should log with level. It can swith a module log and control
	 * the log of specify module.
	 * @example
	 *     std::vector<LogLevel> module_levels(MAX_MODULE_SIZE, LogLevel::DEBUG);
	 *     module_levels[TEST_MODULE] = LogLevel::OFF;
	 *     logger.set_module_should_log_handler([&module_levels](LogLevel level, std::uint16_t module_id)
	 *     {
	 *         return module_levels[module_id] >= level;
	 *     });
	 */
	void set_module_should_log_handler(ModuleShouldLogHandler should_log_handler)
	{
		m_module_should_log = should_log_handler;
	}

	/**
	 * @note Cannot pass @c fmt as nullptr or is ambiguous with another call function.
	 */
	template <typename ... Args>
	void log(LogLevel level, const char* fmt, const Args& ... args)
	{
		log(level, 1, invalid_source_location(), fmt, args ...);
	}

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


	void log(LogLevel level, const char* str)
	{
		log(level, 1, invalid_source_location(), str);
	}

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
	void log(LogLevel level, const T& value)
	{
		log(level, 1, invalid_source_location(), value);
	}

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

	/**
	 * @note Cannot pass @c module_id as 0 or is ambiguous with another call function.
	 */
	template <typename ... Args>
	void log(LogLevel level,
			 std::uint16_t module_id,
			 const SourceLocation& location,
			 const char* fmt,
			 const Args& ... args);

	/**
	 * @note Cannot pass @c module_id as 0 or is ambiguous with another call function.
	 */
	void log(LogLevel level,
			 std::uint16_t module_id,
			 const SourceLocation& location,
			 const char* str);

	/**
	 * @note Cannot pass @c module_id as 0 or is ambiguous with another call function.
	 */
	template <typename T>
	void log(LogLevel level,
			 std::uint16_t module_id,
			 const SourceLocation& location,
			 const T& value);

private:
	bool should_log(LogLevel level, std::uint16_t module_id) const
	{
		bool should = m_level <= level;
		if (should && m_module_should_log)
		{
			return m_module_should_log(level, module_id);
		}
		return should;
	}

	void generate_signature(LogLevel level, std::uint16_t module_id);

	void recore_location(const SourceLocation& location)
	{
		if (is_record_location() && is_valid(location))
		{
			m_writer.write(" [{}:{}][{}]", location.file(), location.line(), location.function());
		}
	}

	std::string m_name;
	LogLevel m_level = LogLevel::INFO;
	bool m_record_location = true;
	bool m_record_module = true;
	ModuleNameHandler m_module_name_handler;
	ModuleShouldLogHandler m_module_should_log;
	LogSinkPtr m_sink_ptr;
	char m_write_target[WRITER_BUFFER_SIZE_DEFAULT];
	TextWriter m_writer;
};


template <typename ... Args>
void TextLogger::log(LogLevel level,
						   std::uint16_t module_id,
						   const SourceLocation& location,
						   const char* fmt,
						   const Args& ... args)
{
	if (this->should_log(level, module_id))
	{
		m_writer.clear();
		this->generate_signature(level, module_id);
		m_writer.write(fmt, args ...);
		recore_location(location);
		m_writer.append(LIGHTS_LINE_ENDER);
		m_sink_ptr->write(m_writer.str_view());
	}
}


template <typename T>
void TextLogger::log(LogLevel level, std::uint16_t module_id, const SourceLocation& location, const T& value)
{
	if (this->should_log(level, module_id))
	{
		m_writer.clear();
		this->generate_signature(level, module_id);
		m_writer << value;
		recore_location(location);
		m_writer.append(LIGHTS_LINE_ENDER);
		m_sink_ptr->write(m_writer.str_view());
	}
}


/**
 * Binary log message header.
 */
struct BinaryMessageSignature
{
public:
	struct AlignPart
	{
		PreciseTime time;
		std::uint32_t file_id;
		std::uint32_t function_id;
		std::uint32_t line;
		std::uint32_t description_id;
	};

	/**
	 * To avoid memory alignment to save ouput content.
	 * @note And must ensure this have enought memory to hold all memeber in UnalignPartInterface.
	 *       So all change in UnalignPartInterface should consider change this.
	 */
	struct UnalignPartStorage
	{
		std::uint8_t entity[7];
	};

	struct UnalignPartInterface
	{
		std::uint16_t log_id;
		std::uint16_t module_id;
		std::uint16_t argument_length;
		LogLevel level;
	};

	PreciseTime get_time() const
	{
		return m_align_part.time;
	}

	void set_time(const PreciseTime& time)
	{
		m_align_part.time = time;
	}

	std::uint32_t get_file_id() const
	{
		return m_align_part.file_id;
	}

	void set_file_id(std::uint32_t file_id)
	{
		m_align_part.file_id = file_id;
	}

	std::uint32_t get_function_id() const
	{
		return m_align_part.function_id;
	}

	void set_function_id(std::uint32_t function_id)
	{
		m_align_part.function_id = function_id;
	}

	std::uint32_t get_line() const
	{
		return m_align_part.line;
	}

	void set_line(std::uint32_t line)
	{
		m_align_part.line = line;
	}

	std::uint32_t get_description_id() const
	{
		return m_align_part.description_id;
	}

	void set_description_id(std::uint32_t description_id)
	{
		m_align_part.description_id = description_id;
	}

	std::uint16_t get_log_id() const
	{
		return unalign_part_interface()->log_id;
	}

	void set_log_id(std::uint16_t log_id)
	{
		unalign_part_interface()->log_id = log_id;
	}

	std::uint16_t get_module_id() const
	{
		return unalign_part_interface()->module_id;
	}

	void set_module_id(std::uint16_t module_id)
	{
		unalign_part_interface()->module_id = module_id;
	}

	std::uint16_t get_argument_length() const
	{
		return unalign_part_interface()->argument_length;
	}

	void set_argument_length(std::uint16_t argument_length)
	{
		unalign_part_interface()->argument_length = argument_length;
	}

	LogLevel get_level() const
	{
		return unalign_part_interface()->level;
	}

	void set_level(LogLevel level)
	{
		unalign_part_interface()->level = level;
	}

	std::size_t get_memory_size() const
	{
		return sizeof(m_align_part) + sizeof(m_unalign_part);
	}

private:
	UnalignPartInterface* unalign_part_interface()
	{
		return reinterpret_cast<UnalignPartInterface*>(&m_unalign_part);
	}

	const UnalignPartInterface* unalign_part_interface() const
	{
		return reinterpret_cast<const UnalignPartInterface*>(&m_unalign_part);
	}

	AlignPart m_align_part;
	UnalignPartStorage m_unalign_part;
};


/**
 * Logs message with binary mode to the backend sink. Binary log message is optimized with output,
 * so can save output and record more information. On the other hand, binary log message is
 * structured and can be convenient analyse.
 */
class BinaryLogger
{
public:
	using ModuleShouldLogHandler = std::function<bool(LogLevel, std::uint16_t)>;

	BinaryLogger(std::uint16_t log_id, LogSinkPtr sink_ptr, StringTablePtr str_table_ptr);

	std::uint16_t get_log_id() const
	{
		return m_signature.get_log_id();
	}

	LogLevel get_level() const
	{
		return m_level;
	}

	/**
	 * Sets the level of this logger.
	 */
	void set_level(LogLevel level)
	{
		m_level = level;
	}

	ModuleShouldLogHandler get_module_should_log_handler() const
	{
		return m_module_should_log;
	}

	/**
	 * Sets the module should log with level. It can swith a module log and control
	 * the log of specify module.
	 * @example
	 *     std::vector<LogLevel> module_levels(MAX_MODULE_SIZE, LogLevel::DEBUG);
	 *     module_levels[TEST_MODULE] = LogLevel::OFF;
	 *     logger.set_module_should_log_handler([&module_levels](LogLevel level, std::uint16_t module_id)
	 *     {
	 *         return module_levels[module_id] >= level;
	 *     });
	 */
	void set_module_should_log_handler(ModuleShouldLogHandler should_log_handler)
	{
		m_module_should_log = should_log_handler;
	}

	template <typename ... Args>
	void log(LogLevel level,
			 std::uint16_t module_id,
			 const SourceLocation& location,
			 const char* fmt,
			 const Args& ... args);

	void log(LogLevel level,
			 std::uint16_t module_id,
			 const SourceLocation& location,
			 const char* str);

	template <typename T>
	void log(LogLevel level,
			 std::uint16_t module_id,
			 const SourceLocation& location,
			 const T& value);

private:
	bool should_log(LogLevel level, std::uint16_t module_id) const
	{
		bool should = m_level <= level;
		if (should && m_module_should_log)
		{
			return m_module_should_log(level, module_id);
		}
		return should;
	}

	void generate_signature(LogLevel level,
							std::uint16_t module_id,
							const SourceLocation& location,
							StringView description)
	{
		m_signature.set_time(get_precise_time());
		auto file_id = m_str_table_ptr->get_index(location.file());
		m_signature.set_file_id(static_cast<std::uint32_t>(file_id));
		auto function_id = m_str_table_ptr->get_index(location.function());
		m_signature.set_function_id(static_cast<std::uint32_t>(function_id));
		m_signature.set_line(location.line());
		m_signature.set_description_id(static_cast<std::uint32_t>(m_str_table_ptr->get_index(description)));
		m_signature.set_module_id(module_id);
		m_signature.set_level(level);
	}

	LogLevel m_level = LogLevel::INFO;
	ModuleShouldLogHandler m_module_should_log;
	LogSinkPtr m_sink_ptr;
	StringTablePtr m_str_table_ptr;
	BinaryMessageSignature m_signature;
	char m_write_target[WRITER_BUFFER_SIZE_LARGE];
	BinaryStoreWriter m_writer;
};


template <typename ... Args>
void BinaryLogger::log(LogLevel level,
							 std::uint16_t module_id,
							 const SourceLocation& location,
							 const char* fmt,
							 const Args& ... args)
{
	if (this->should_log(level, module_id))
	{
		generate_signature(level, module_id, location, fmt);

		m_writer.clear();
		m_writer.write(fmt, args ...);
		m_signature.set_argument_length(static_cast<std::uint16_t>(m_writer.length()));

		m_sink_ptr->write({&m_signature, m_signature.get_memory_size()});
		m_sink_ptr->write(m_writer.str_view());
	}
}


template <typename T>
void BinaryLogger::log(LogLevel level,
							 std::uint16_t module_id,
							 const SourceLocation& location,
							 const T& value)
{
	if (this->should_log(level, module_id))
	{
		StringView description = "{}";
		generate_signature(level, module_id, location, description);

		m_writer.clear();
		m_writer.write(description, value);
		m_signature.set_argument_length(static_cast<std::uint16_t>(m_writer.length()));

		m_sink_ptr->write({&m_signature, m_signature.get_memory_size()});
		m_sink_ptr->write(m_writer.str_view());
	}
}


class BinaryLogReader
{
public:
	BinaryLogReader(StringView log_filename, StringTablePtr str_table_ptr) :
		m_file(log_filename, "rb"),
		m_str_table_ptr(str_table_ptr),
		m_writer(make_string(m_write_target), str_table_ptr)
	{
	}

	/**
	 * @note Return nullptr when have no log message to read.
	 */
	StringView read();

	/**
	 * @note Line is start at 0.
	 */
	void jump(std::size_t line);

	bool eof()
	{
		m_file.peek();
		return m_file.eof();
	}

private:
	FileStream m_file;
	StringTablePtr m_str_table_ptr;
	BinaryMessageSignature m_signature;
	char m_write_target[WRITER_BUFFER_SIZE_LARGE];
	BinaryRestoreWriter m_writer;
};


#ifdef LIGHTS_OPEN_LOG
#	define LIGHTS_LOG(logger, level, module, ...) \
		logger.log(level, module, LIGHTS_CURRENT_SOURCE_LOCATION, __VA_ARGS__);
#else
#	define LIGHTS_LOG(logger, level, module, ...)
#endif

/**
 * Unified interface of logger to log message.
 * @param ... Can use format string and arguments or just a any type value.
 */
#define LIGHTS_DEBUG(logger, module, ...) \
	LIGHTS_LOG(logger, lights::LogLevel::DEBUG, module, __VA_ARGS__);
#define LIGHTS_INFO(logger, module, ...) \
	LIGHTS_LOG(logger, lights::LogLevel::INFO, module, __VA_ARGS__);
#define LIGHTS_WARN(logger, module, ...) \
	LIGHTS_LOG(logger, lights::LogLevel::WARN, module, __VA_ARGS__);
#define LIGHTS_ERROR(logger, module, ...) \
	LIGHTS_LOG(logger, lights::LogLevel::ERROR, module, __VA_ARGS__);


} // namespace lights
