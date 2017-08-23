/**
 * logger.h
 * @author wherewindblow
 * @date   Dec 09, 2016
 */

#pragma once

#include <cstring>
#include <ctime>
#include <memory>
#include <vector>
#include <unordered_map>
#include <fstream>

#include <time.h>

#include "format.h"
#include "file.h"
#include "exception.h"


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

struct PreciseTime
{
	PreciseTime() = default;

	PreciseTime(std::int64_t seconds, std::int64_t nanoseconds):
		seconds(seconds), nanoseconds(nanoseconds)
	{}

	std::int64_t seconds;
	std::int64_t nanoseconds;
};

PreciseTime get_precise_time();


/**
 * Logger message to the backend sink.
 * @tparam Sink  Support `void write(const void* buf, std::size_t len);`
 *
 * Message is compose by a signature header and message body.
 * Signature is compose by time, logger name and log level.
 */
template <typename Sink>
class TextLogger
{
public:
	using ModuleNameHandler = std::function<StringView(std::uint16_t)>;
	using ModuleShouldLogHandler = std::function<bool(LogLevel, std::uint16_t)>;

	TextLogger(StringView name, std::shared_ptr<Sink> sink);

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

	bool is_record_location() const
	{
		return m_record_location;
	}

	void set_record_location(bool enable_record)
	{
		m_record_location = enable_record;
	}

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

	void set_module_name_handler(ModuleNameHandler module_name_handler)
	{
		m_module_name_handler = module_name_handler;
	}

	ModuleShouldLogHandler get_module_should_log_handler() const
	{
		return m_module_should_log;
	}

	/**
	 * Set the module should log with level. It can swith a module log and control
	 * the log of specify module.
	 * @param should_log_handler  It's a handler that pass level and module_id and
	 *                            return this message should log.
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
	 * @note cannot pass @c fmt as nullptr or is ambiguous with another call function.
	 */
	template <typename ... Args>
	void log(LogLevel level, const char* fmt, const Args& ... args)
	{
		log(level, 1, LIGHTS_CURRENT_SOURCE_LOCATION, fmt, args ...);
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
		log(level, 1, LIGHTS_CURRENT_SOURCE_LOCATION, str);
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
		log(level, 1, LIGHTS_CURRENT_SOURCE_LOCATION, value);
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
	 * @note cannot pass @c module_id as 0 or is ambiguous with another call function.
	 */
	template <typename ... Args>
	void log(LogLevel level,
			 std::uint16_t module_id,
			 const SourceLocation& location,
			 const char* fmt,
			 const Args& ... args);

	/**
	 * @note cannot pass @c module_id as 0 or is ambiguous with another call function.
	 */
	void log(LogLevel level,
			 std::uint16_t module_id,
			 const SourceLocation& location,
			 const char* str);

	/**
	 * @note cannot pass @c module_id as 0 or is ambiguous with another call function.
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
		if (is_record_location())
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
	std::shared_ptr<Sink> m_sink;
	TextWriter<> m_writer;
};


template <typename Sink>
TextLogger<Sink>::TextLogger(StringView name, std::shared_ptr<Sink> sink) :
	m_name(name.data()), m_sink(sink) {}


template <typename Sink>
template <typename ... Args>
void TextLogger<Sink>::log(LogLevel level,
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
		m_sink->write(m_writer.str_view());
	}
}

template <typename Sink>
void TextLogger<Sink>::log(LogLevel level, std::uint16_t module_id, const SourceLocation& location, const char* str)
{
	if (this->should_log(level, module_id))
	{
		m_writer.clear();
		this->generate_signature(level, module_id);
		m_writer.append(str);
		recore_location(location);
		m_writer.append(LIGHTS_LINE_ENDER);
		m_sink->write(m_writer.str_view());
	}
}

template <typename Sink>
template <typename T>
void TextLogger<Sink>::log(LogLevel level, std::uint16_t module_id, const SourceLocation& location, const T& value)
{
	if (this->should_log(level, module_id))
	{
		m_writer.clear();
		this->generate_signature(level, module_id);
		m_writer << value;
		recore_location(location);
		m_writer.append(LIGHTS_LINE_ENDER);
		m_sink->write(m_writer.str_view());
	}
}

/**
 * Use second as a tick to record timestamp.
 * It's faster than use chrono, but precision is not enought.
 */
//template <typename Sink>
//void TextLogger<Sink>::generate_signature()
//{
//	std::time_t time = std::time(nullptr);
//	std::tm tm;
//	localtime_r(&time, &tm);
//
//	m_writer << '[' << Timestamp(time);
//	m_writer << "] [" << m_name << "] [" << to_string(m_level) << "] ";
//}

template <typename Sink>
void TextLogger<Sink>::generate_signature(LogLevel level, std::uint16_t module_id)
{
	PreciseTime precise_time = get_precise_time();
	m_writer << '[' << Timestamp(precise_time.seconds) << '.';

	auto millis = precise_time.nanoseconds / 1000 / 1000;
	m_writer << pad(static_cast<unsigned>(millis), '0', 3);
	m_writer << "] [" << m_name << "] ";
	if (is_record_module())
	{
		if (m_module_name_handler)
		{
			m_writer << "[" << m_module_name_handler(module_id) << "] ";
		}
		else
		{
			m_writer << "[" << module_id << "] ";
		}
	}
	m_writer << "[" << to_string(level) << "] ";
}


class StringTable
{
public:
	using StringViewPtr = std::shared_ptr<const StringView>;
	using StringTablePtr = std::shared_ptr<StringTable>;

	static StringTablePtr create(StringView filename)
	{
		return std::make_shared<StringTable>(filename);
	}

	StringTable(StringView filename);

	~StringTable();

	std::size_t get_str_index(StringView str);

	std::size_t add_str(StringView str);

	StringView operator[] (std::size_t index) const
	{
		return *m_str_array[index];
	}

	StringView get_str(std::size_t index) const
	{
		return (*this)[index];
	}

private:
	static StringTablePtr instance_ptr;

	struct StringHash
	{
		size_t operator()(const StringViewPtr& str) const noexcept
		{
			return std::_Hash_impl::hash(str->data(), str->length());
		}
	};

	struct StringEqualTo
	{
		bool operator()(const StringViewPtr& lhs, const StringViewPtr& rhs) const noexcept
		{
			if (lhs->length() != rhs->length())
			{
				return false;
			}
			else
			{
				return std::memcmp(lhs->data(), rhs->data(), rhs->length()) == 0;
			}
		}
	};

	struct EmptyDeleter
	{
		void operator()(const StringView*) const noexcept {}
	};

	struct StringDeleter
	{
		void operator()(const StringView* str) const noexcept
		{
			delete[] str->data();
			delete str;
		}
	};

	std::fstream m_file;
	std::size_t m_last_index = static_cast<std::size_t>(-1);
	std::vector<StringViewPtr> m_str_array; // To generate index
	std::unordered_map<StringViewPtr,
					   std::uint32_t,
					   StringHash,
					   StringEqualTo> m_str_hash; // To find faster.
};

using StringTablePtr = StringTable::StringTablePtr;


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
 * Logger message to the backend sink.
 * @tparam Sink  Support `void write(const void* buf, std::size_t len);`
 */
template <typename Sink>
class BinaryLogger
{
public:
	using ModuleShouldLogHandler = std::function<bool(LogLevel, std::uint16_t)>;

	BinaryLogger(std::uint16_t log_id, std::shared_ptr<Sink> sink, StringTablePtr str_table);

	std::uint16_t get_log_id() const
	{
		return m_signature.get_log_id();
	}

	LogLevel get_level() const
	{
		return m_level;
	}

	void set_level(LogLevel level)
	{
		m_level = level;
	}

	ModuleShouldLogHandler get_module_should_log_handler() const
	{
		return m_module_should_log;
	}

	/**
	 * Set the module should log with level. It can swith a module log and control
	 * the log of specify module.
	 * @param should_log_handler  It's a handler that pass level and module_id and
	 *                            return this message should log.
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
							StringView descript)
	{
		m_signature.set_time(get_precise_time());
		auto file_id = m_str_table->get_str_index(location.file());
		m_signature.set_file_id(static_cast<std::uint32_t>(file_id));
		auto function_id = m_str_table->get_str_index(location.function());
		m_signature.set_function_id(static_cast<std::uint32_t>(function_id));
		m_signature.set_line(location.line());
		m_signature.set_description_id(static_cast<std::uint32_t>(m_str_table->get_str_index(descript)));
		m_signature.set_module_id(module_id);
		m_signature.set_level(level);
	}

	LogLevel m_level = LogLevel::INFO;
	ModuleShouldLogHandler m_module_should_log;
	std::shared_ptr<Sink> m_sink;
	StringTablePtr m_str_table;
	BinaryMessageSignature m_signature;
	BinaryStoreWriter<WRITER_BUFFER_SIZE_LARGE> m_writer;
};


template <typename Sink>
BinaryLogger<Sink>::BinaryLogger(std::uint16_t log_id, std::shared_ptr<Sink> sink, StringTablePtr str_table) :
	m_sink(sink), m_str_table(str_table)
{
	m_signature.set_log_id(log_id);
}


template <typename Sink>
template <typename ... Args>
void BinaryLogger<Sink>::log(LogLevel level,
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

		m_sink->write({&m_signature, m_signature.get_memory_size()});
		m_sink->write(m_writer.str_view());
	}
}


template <typename Sink>
void BinaryLogger<Sink>::log(LogLevel level,
							 std::uint16_t module_id,
							 const SourceLocation& location,
							 const char* str)
{
	if (this->should_log(level, module_id))
	{
		generate_signature(level, module_id, location, str);
		m_signature.set_argument_length(0);
		m_sink->write({&m_signature, m_signature.get_memory_size()});
	}
}


template <typename Sink>
template <typename T>
void BinaryLogger<Sink>::log(LogLevel level,
							 std::uint16_t module_id,
							 const SourceLocation& location,
							 const T& value)
{
	if (this->should_log(level, module_id))
	{
		generate_signature(level, module_id, location, "{}");

		m_writer.clear();
		m_writer.write("{}", value);
		m_signature.set_argument_length(static_cast<std::uint16_t>(m_writer.length()));

		m_sink->write({&m_signature, m_signature.get_memory_size()});
		m_sink->write(m_writer.str_view());
	}
}


class BinaryLogReader
{
public:
	BinaryLogReader(StringView log_filename, StringTablePtr str_table) :
		m_file(log_filename, "rb"), m_str_table(str_table)
	{
	}

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
	StringTablePtr m_str_table;
	BinaryMessageSignature m_signature;
	BinaryRestoreWriter<WRITER_BUFFER_SIZE_LARGE> m_writer;
};


#ifdef LIGHTS_OPEN_LOG
#	define LIGHTS_LOG(logger, level, module, ...) \
		logger.log(level, module, LIGHTS_CURRENT_SOURCE_LOCATION, __VA_ARGS__);
#else
#	define LIGHTS_LOG(logger, level, module, ...)
#endif

#define LIGHTS_DEBUG(logger, module, ...) \
	LIGHTS_LOG(logger, lights::LogLevel::DEBUG, module, __VA_ARGS__);
#define LIGHTS_INFO(logger, module, ...) \
	LIGHTS_LOG(logger, lights::LogLevel::INFO, module, __VA_ARGS__);
#define LIGHTS_WARN(logger, module, ...) \
	LIGHTS_LOG(logger, lights::LogLevel::WARN, module, __VA_ARGS__);
#define LIGHTS_ERROR(logger, module, ...) \
	LIGHTS_LOG(logger, lights::LogLevel::ERROR, module, __VA_ARGS__);


} // namespace lights
