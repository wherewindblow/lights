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


namespace lights {

namespace details {

static const std::string log_level_names[] = {
	"debug", "info", "warnning", "error", "off"
};

} // namespace details


enum class LogLevel: unsigned char
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
TextLogger<Sink>::TextLogger(const std::string& name, std::shared_ptr<Sink> sink) :
	m_name(name), m_sink(sink) {}


template <typename Sink>
template <typename ... Args>
void TextLogger<Sink>::log(LogLevel level, const char* fmt, const Args& ... args)
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
void TextLogger<Sink>::log(LogLevel level, const char* str)
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
void TextLogger<Sink>::log(LogLevel level, const T& value)
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
//void TextLogger<Sink>::generate_signature()
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
void TextLogger<Sink>::generate_signature_header()
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


/**
 * Logger message to the backend sink.
 * @tparam Sink  Support `void write(const char* str, std::size_t len);`
 */
template <typename Sink>
class BinaryLogger
{
public:
	struct TimeValue
	{
		std::int64_t seconds;
		std::int64_t nanoseconds;
	};

	struct AlignPart
	{
		TimeValue time;
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

	struct SignatureHeader
	{
		AlignPart align_part;
		UnalignPartStorage unalign_part;
	};


	BinaryLogger(std::uint16_t log_id, std::shared_ptr<Sink> sink);

	std::uint16_t get_log_id() const
	{
		UnalignPartInterface* unalign = reinterpret_cast<UnalignPartInterface*>(&m_signature.unalign_part);
		return unalign->log_id;
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

	TimeValue get_time_value() const
	{
		namespace chrono = std::chrono;
		auto chrono_time = chrono::system_clock::now();
		std::time_t seconds = chrono::system_clock::to_time_t(chrono_time);
		auto duration = chrono_time.time_since_epoch();
		using target_time_type = chrono::nanoseconds;
		auto nano = chrono::duration_cast<target_time_type>(duration).count() % target_time_type::period::den;
		return TimeValue { seconds, nano };
	}

	void generate_signature(LogLevel level,
							std::uint16_t module_id,
							const char* file,
							const char* function,
							std::uint32_t line,
							const char* descript)
	{
		m_signature.align_part.time = get_time_value();
		m_signature.align_part.file_id = static_cast<decltype(m_signature.align_part.file_id)>(StringTable::instance().get_str_index(file));
		m_signature.align_part.function_id = static_cast<decltype(m_signature.align_part.function_id)>(StringTable::instance().get_str_index(function));
		m_signature.align_part.line = line;
		m_signature.align_part.description_id = static_cast<decltype(m_signature.align_part.description_id)>(StringTable::instance().get_str_index(descript));

		UnalignPartInterface* unalign = reinterpret_cast<UnalignPartInterface*>(&m_signature.unalign_part);
		unalign->module_id = module_id;
		unalign->level = level;
	}

	void write_signature() const
	{
		m_sink->write(reinterpret_cast<const char*>(&m_signature),
					  sizeof(m_signature.align_part) + sizeof(m_signature.unalign_part));
	}

	LogLevel m_level = LogLevel::INFO;
	std::shared_ptr<Sink> m_sink;
	SignatureHeader m_signature;

	static constexpr std::size_t MAX_MESSAGE_SIZE = 1000;
	BinaryStoreWriter<MAX_MESSAGE_SIZE> m_writer;
};


template <typename Sink>
BinaryLogger<Sink>::BinaryLogger(std::uint16_t log_id, std::shared_ptr<Sink> sink) :
	m_sink(sink)
{
	UnalignPartInterface* unalign = reinterpret_cast<UnalignPartInterface*>(&m_signature.unalign_part);
	unalign->log_id = log_id;
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
		UnalignPartInterface* unalign = reinterpret_cast<UnalignPartInterface*>(&m_signature.unalign_part);
		unalign->argument_length = static_cast<decltype(unalign->argument_length)>(m_writer.length());

		write_signature();
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
		UnalignPartInterface* unalign = reinterpret_cast<UnalignPartInterface*>(&m_signature.unalign_part);
		unalign->argument_length = 0;
		write_signature();
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
		UnalignPartInterface* unalign = reinterpret_cast<UnalignPartInterface*>(&m_signature.unalign_part);
		unalign->argument_length = static_cast<decltype(unalign->argument_length)>(m_writer.length());

		write_signature();
		auto view = m_writer.str_view();
		m_sink->write(view.string, view.length);
	}
}

} // namespace lights
