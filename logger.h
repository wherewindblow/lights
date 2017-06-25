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
#include <vector>
#include <unordered_map>
#include <fstream>

#include <time.h>

#include "format.h"
#include "file.h"


namespace lights {

namespace details {

static const std::string log_level_names[] = {
	"debug", "info", "warnning", "error", "off"
};

} // namespace details


enum class LogLevel: std::uint8_t
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
class TextLogger
{
public:
	TextLogger(const std::string& name, std::shared_ptr<Sink> sink);

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

	void generate_signature();

	std::string m_name;
	LogLevel m_level = LogLevel::INFO;
	std::shared_ptr<Sink> m_sink;
	MemoryWriter<> m_writer;
};


template <typename Sink>
TextLogger<Sink>::TextLogger(const std::string& name, std::shared_ptr<Sink> sink) :
	m_name(name), m_sink(sink) {}


template <typename Sink>
template <typename ... Args>
void TextLogger<Sink>::log(LogLevel level, const char* fmt, const Args& ... args)
{
	if (this->should_log(level))
	{
		m_writer.clear();
		this->generate_signature();
		m_writer.write(fmt, args ...);
		m_writer.append('\n');
		lights::StringView view = m_writer.str_view();
		m_sink->write(view.string, view.length);
	}
}


template <typename Sink>
void TextLogger<Sink>::log(LogLevel level, const char* str)
{
	if (this->should_log(level))
	{
		m_writer.clear();
		this->generate_signature();
		m_writer << str << '\n';
		lights::StringView view = m_writer.str_view();
		m_sink->write(view.string, view.length);
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
		lights::StringView view = m_writer.str_view();
		m_sink->write(view.string, view.length);
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
	namespace chrono = std::chrono;
	auto chrono_time = chrono::system_clock::now();
	std::time_t time = chrono::system_clock::to_time_t(chrono_time);

	m_writer << '[' << Timestamp(time) << '.';

	auto duration = chrono_time.time_since_epoch();
	auto millis = chrono::duration_cast<chrono::milliseconds>(duration).count() % 1000;
	m_writer << pad(static_cast<unsigned>(millis), '0', 3);

	m_writer << "] [" << m_name << "] [" << to_string(m_level) << "] ";
}


namespace details {

template <typename T = void>
class StringTableImpl
{
public:
	using StringViewPtr = std::shared_ptr<const StringView>;

	static StringTableImpl& instance()
	{
		return *instance_ptr;
	}

	static void init_instance(const std::string& filename)
	{
		instance_ptr = std::make_unique<StringTableImpl>(filename);
	}

	StringTableImpl(const std::string& filename);

	~StringTableImpl();

	std::size_t get_str_index(const StringView& view)
	{
		StringViewPtr str_ptr(&view, EmptyDeleter());
		auto itr = m_str_hash.find(str_ptr);
		if (itr == m_str_hash.end())
		{
			return add_str(view);
		}
		else
		{
			return itr->second;
		}
	}

	std::size_t get_str_index(const char* str)
	{
		StringView view = { str, std::strlen(str) };
		return get_str_index(view);
	}

	std::size_t get_str_index(const std::string& str)
	{
		StringView view = { str.c_str(), str.length() };
		return get_str_index(view);
	}

	std::size_t add_str(const StringView& view)
	{
		char* storage = new char[view.length];
		std::memcpy(storage, view.string, view.length);

		auto str_ptr = std::make_shared<StringView>(storage, view.length);
		m_str_array.push_back(str_ptr);
		auto pair = std::make_pair(str_ptr, m_str_array.size() - 1);
		m_str_hash.insert(pair);
		return pair.second;
	}

	StringView operator[] (std::size_t index) const
	{
		return *m_str_array[index];
	}

private:
	static std::unique_ptr<StringTableImpl> instance_ptr;

	struct StringHash
	{
		size_t operator()(const StringViewPtr& str) const noexcept
		{
			return std::_Hash_impl::hash(str->string, str->length);
		}
	};

	struct StringEqualTo
	{
		bool operator()(const StringViewPtr& lhs, const StringViewPtr& rhs) const noexcept
		{
			if (lhs->length != rhs->length)
			{
				return false;
			}
			else
			{
				return std::memcmp(lhs->string, rhs->string, rhs->length) == 0;
			}
		}
	};

	struct EmptyDeleter
	{
		void operator()(const StringView*) const noexcept {}
	};

	struct StringDeleter
	{
		void operator()(const StringView* view) const noexcept
		{
			delete[] view->string;
			delete view;
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


template <typename T>
std::unique_ptr<StringTableImpl<T>> StringTableImpl<T>::instance_ptr = nullptr;


template <typename T>
StringTableImpl<T>::StringTableImpl(const std::string& filename)
{
	m_file.open(filename);
	if (m_file.is_open())
	{
		std::string line;
		while (std::getline(m_file, line))
		{
			add_str(StringView(line.c_str(), line.length()));
		}
		m_last_index = m_str_array.size() - 1;
	}
	else
	{
		m_file.open(filename, std::ios_base::out); // Create file.
		if (!m_file.is_open())
		{
			throw std::runtime_error(format("StringTableImpl: cannot open file: \"{}\"", filename));
		}
	}
}


template <typename T>
StringTableImpl<T>::~StringTableImpl()
{
	if (m_file.is_open())
	{
		m_file.seekp(0, std::ios_base::end);
		m_file.clear();
		for (std::size_t i = m_last_index + 1; m_file && i < m_str_array.size(); ++i)
		{
			m_file.write(m_str_array[i]->string, m_str_array[i]->length) << '\n';
		}
		m_file.close();
	}
}

} // namespace details

using StringTable = details::StringTableImpl<>;


static constexpr std::size_t MAX_BINARY_MESSAGE_SIZE = 1000;


struct PreciseTime
{
	std::int64_t seconds;
	std::int64_t nanoseconds;
};

inline PreciseTime get_precise_time()
{
	namespace chrono = std::chrono;
	auto chrono_time = chrono::system_clock::now();
	std::time_t seconds = chrono::system_clock::to_time_t(chrono_time);
	auto duration = chrono_time.time_since_epoch();
	using target_time_type = chrono::nanoseconds;
	auto nano = chrono::duration_cast<target_time_type>(duration).count() % target_time_type::period::den;
	return PreciseTime { seconds, nano };
}


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
		return align_part.time;
	}

	void set_time(const PreciseTime& time)
	{
		align_part.time = time;
	}

	std::uint32_t get_file_id() const
	{
		return align_part.file_id;
	}

	void set_file_id(std::uint32_t file_id)
	{
		align_part.file_id = file_id;
	}

	std::uint32_t get_function_id() const
	{
		return align_part.function_id;
	}

	void set_function_id(std::uint32_t function_id)
	{
		align_part.function_id = function_id;
	}

	std::uint32_t get_line() const
	{
		return align_part.line;
	}

	void set_line(std::uint32_t line)
	{
		align_part.line = line;
	}

	std::uint32_t get_description_id() const
	{
		return align_part.description_id;
	}

	void set_description_id(std::uint32_t description_id)
	{
		align_part.description_id = description_id;
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
		return sizeof(align_part) + sizeof(unalign_part);
	}

private:
	UnalignPartInterface* unalign_part_interface()
	{
		return reinterpret_cast<UnalignPartInterface*>(&unalign_part);
	}

	const UnalignPartInterface* unalign_part_interface() const
	{
		return reinterpret_cast<const UnalignPartInterface*>(&unalign_part);
	}

	AlignPart align_part;
	UnalignPartStorage unalign_part;
};


/**
 * Logger message to the backend sink.
 * @tparam Sink  Support `void write(const char* str, std::size_t len);`
 */
template <typename Sink>
class BinaryLogger
{
public:
	BinaryLogger(std::uint16_t log_id, std::shared_ptr<Sink> sink);

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
			 const char* file,
			 const char* function,
			 std::uint32_t line,
			 const char* fmt,
			 const Args& ... args);


	void log(LogLevel level,
			 std::uint16_t module_id,
			 const char* file,
			 const char* function,
			 std::uint32_t line,
			 const char* str);


	template <typename T>
	void log(LogLevel level,
			 std::uint16_t module_id,
			 const char* file,
			 const char* function,
			 std::uint32_t line,
			 const T& value);

private:
	bool should_log(LogLevel level) const
	{
		return m_level <= level;
	}

	void generate_signature(LogLevel level,
							std::uint16_t module_id,
							const char* file,
							const char* function,
							std::uint32_t line,
							const char* descript)
	{
		m_signature.set_time(get_precise_time());
		StringTable& str_table = StringTable::instance();
		m_signature.set_file_id(static_cast<std::uint32_t>(str_table.get_str_index(file)));
		m_signature.set_function_id(static_cast<std::uint32_t>(str_table.get_str_index(function)));
		m_signature.set_line(line);
		m_signature.set_description_id(static_cast<std::uint32_t>(str_table.get_str_index(descript)));
		m_signature.set_module_id(module_id);
		m_signature.set_level(level);
	}

	LogLevel m_level = LogLevel::INFO;
	std::shared_ptr<Sink> m_sink;
	BinaryMessageSignature m_signature;

	BinaryStoreWriter<MAX_BINARY_MESSAGE_SIZE> m_writer;
};


template <typename Sink>
BinaryLogger<Sink>::BinaryLogger(std::uint16_t log_id, std::shared_ptr<Sink> sink) :
	m_sink(sink)
{
	m_signature.set_log_id(log_id);
}


template <typename Sink>
template <typename ... Args>
void BinaryLogger<Sink>::log(LogLevel level,
							 std::uint16_t module_id,
							 const char* file,
							 const char* function,
							 std::uint32_t line,
							 const char* fmt,
							 const Args& ... args)
{
	if (this->should_log(level))
	{
		generate_signature(level, module_id, file, function, line, fmt);

		m_writer.clear();
		m_writer.write(fmt, args ...);
		m_signature.set_argument_length(static_cast<std::uint16_t>(m_writer.length()));

		m_sink->write(reinterpret_cast<const char*>(&m_signature), m_signature.get_memory_size());
		auto view = m_writer.str_view();
		m_sink->write(view.string, view.length);
	}
}


template <typename Sink>
void BinaryLogger<Sink>::log(LogLevel level,
							 std::uint16_t module_id,
							 const char* file,
							 const char* function,
							 std::uint32_t line,
							 const char* str)
{
	if (this->should_log(level))
	{
		generate_signature(level, module_id, file, function, line, str);
		m_signature.set_argument_length(0);
		m_sink->write(reinterpret_cast<const char*>(&m_signature), m_signature.get_memory_size());
	}
}


template <typename Sink>
template <typename T>
void BinaryLogger<Sink>::log(LogLevel level,
							 std::uint16_t module_id,
							 const char* file,
							 const char* function,
							 std::uint32_t line,
							 const T& value)
{
	if (this->should_log(level))
	{
		generate_signature(level, module_id, file, function, line, "{}");

		m_writer.clear();
		m_writer.write("{}", value);
		m_signature.set_argument_length(static_cast<std::uint16_t>(m_writer.length()));

		m_sink->write(reinterpret_cast<const char*>(&m_signature), m_signature.get_memory_size());
		auto view = m_writer.str_view();
		m_sink->write(view.string, view.length);
	}
}


class BinaryLogReader
{
public:
	BinaryLogReader(const std::string& log_filename) :
		m_file(log_filename, "rb")
	{
	}

	const char* read()
	{
		m_writer.clear();
		auto len = m_file.read(&m_signature, m_signature.get_memory_size());
		if (len == 0)
		{
			return nullptr;
		}

		std::unique_ptr<std::uint8_t[]> arguments(new std::uint8_t[m_signature.get_argument_length()]);
		m_file.read(arguments.get(), m_signature.get_argument_length());
		StringTable& str_table = StringTable::instance();

		m_writer.write_text("[{}.{}] [{}] [{}.{}] ",
					   Timestamp(m_signature.get_time().seconds),
					   pad(m_signature.get_time().nanoseconds, '0', 10),
					   to_string(m_signature.get_level()),
					   m_signature.get_log_id(),
					   m_signature.get_module_id());

		m_writer.write_binary(str_table[m_signature.get_description_id()].string,
					   arguments.get(),
					   m_signature.get_argument_length());

		m_writer.write_text("  [{}:{}] [{}]",
					   str_table[m_signature.get_file_id()],
					   m_signature.get_line(),
					   str_table[m_signature.get_function_id()]);

		return m_writer.c_str();
	}

	/**
	 * @note Line is start at 0.
	 */
	void jump_to(std::size_t line)
	{
		for (std::size_t i = 0; i < line; ++i)
		{
			m_file.read(&m_signature, m_signature.get_memory_size());
			auto pos = m_file.tell();
			m_file.seek(pos + m_signature.get_argument_length() + 1, FileSeekWhence::BEGIN);
		}
	}

	bool eof()
	{
		m_file.peek();
		return m_file.eof();
	}

private:
	FileStream m_file;
	BinaryMessageSignature m_signature;
	BinaryRestoreWriter<MAX_BINARY_MESSAGE_SIZE> m_writer;
};

} // namespace lights
