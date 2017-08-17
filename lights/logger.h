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


	template <typename ... Args>
	void log(LogLevel level, StringView fmt, const Args& ... args);

	template <typename ... Args>
	void debug(StringView fmt, const Args& ... args)
	{
		this->log(LogLevel::DEBUG, fmt, args ...);
	}

	template <typename ... Args>
	void info(StringView fmt, const Args& ... args)
	{
		this->log(LogLevel::INFO, fmt, args ...);
	}

	template <typename ... Args>
	void warn(StringView fmt, const Args& ... args)
	{
		this->log(LogLevel::WARN, fmt, args ...);
	}

	template <typename ... Args>
	void error(StringView fmt, const Args& ... args)
	{
		this->log(LogLevel::ERROR, fmt, args ...);
	}


	void log(LogLevel level, StringView str);

	void debug(StringView str)
	{
		this->log(LogLevel::DEBUG, str);
	}

	void info(StringView str)
	{
		this->log(LogLevel::INFO, str);
	}

	void warn(StringView str)
	{
		this->log(LogLevel::WARN, str);
	}

	void error(StringView str)
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

	void generate_signature();

	std::string m_name;
	LogLevel m_level = LogLevel::INFO;
	std::shared_ptr<Sink> m_sink;
	TextWriter<> m_writer;
};


template <typename Sink>
TextLogger<Sink>::TextLogger(StringView name, std::shared_ptr<Sink> sink) :
	m_name(name.data()), m_sink(sink) {}


template <typename Sink>
template <typename ... Args>
void TextLogger<Sink>::log(LogLevel level, StringView fmt, const Args& ... args)
{
	if (this->should_log(level))
	{
		m_writer.clear();
		this->generate_signature();
		m_writer.write(fmt, args ...);
		m_writer.append('\n');
		m_sink->write(m_writer.str_view());
	}
}


template <typename Sink>
void TextLogger<Sink>::log(LogLevel level, StringView str)
{
	if (this->should_log(level))
	{
		m_writer.clear();
		this->generate_signature();
		m_writer << str << '\n';
		m_sink->write(m_writer.str_view());
	}
}


template <typename Sink>
template <typename T>
void TextLogger<Sink>::log(LogLevel level, const T& value)
{
	if (this->should_log(level))
	{
		m_writer.clear();
		this->generate_signature();
		m_writer << value << '\n';
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
void TextLogger<Sink>::generate_signature()
{
	PreciseTime precise_time = get_precise_time();
	m_writer << '[' << Timestamp(precise_time.seconds) << '.';

	auto millis = precise_time.nanoseconds / 1000 / 1000;
	m_writer << pad(static_cast<unsigned>(millis), '0', 3);
	m_writer << "] [" << m_name << "] [" << to_string(m_level) << "] ";
}


class StringTable
{
public:
	using StringViewPtr = std::shared_ptr<const StringView>;
	using StringTablePtr = std::shared_ptr<StringTable>;

	static StringTablePtr& instance()
	{
		return instance_ptr;
	}

	static void init_instance(StringView filename)
	{
		instance_ptr = std::make_shared<StringTable>(filename);
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
	BinaryLogger(std::uint16_t log_id, std::shared_ptr<Sink> sink, StringTablePtr str_table = StringTable::instance());

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
	bool should_log(LogLevel level) const
	{
		return m_level <= level;
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
	if (this->should_log(level))
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
	if (this->should_log(level))
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
	if (this->should_log(level))
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
	BinaryLogReader(StringView log_filename, StringTablePtr str_table = StringTable::instance()) :
		m_file(log_filename, "rb"), m_str_table(str_table)
	{
	}

	StringView read();

	/**
	 * @note Line is start at 0.
	 */
	void jump_to(std::size_t line);

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
