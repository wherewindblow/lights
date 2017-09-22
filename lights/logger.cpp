/**
 * logger.cpp
 * @author wherewindblow
 * @date   Jul 23, 2017
 */

#include "logger.h"

#include <chrono>


namespace lights {

TextLogger::TextLogger(StringView name, LogSinkPtr sink_ptr) :
	m_name(name.data()),
	m_sink_ptr(sink_ptr),
	m_writer(make_string(m_write_target))
{}


void TextLogger::log(LogLevel level, std::uint16_t module_id, const SourceLocation& location, const char* str)
{
	if (this->should_log(level, module_id))
	{
		m_writer.clear();
		this->generate_signature(level, module_id);
		m_writer.append(str);
		recore_location(location);
		m_writer.append(env_end_line());
		m_sink_ptr->write(m_writer.string_view());
	}
}


/**
 * Use second as a tick to record timestamp.
 * It's faster than use chrono, but precision is not enought.
 */

//void TextLogger::generate_signature()
//{
//	std::time_t time = std::time(nullptr);
//	m_writer << '[' << Timestamp(time);
//	m_writer << "] [" << m_name << "] [" << to_string(m_level) << "] ";
//}


void TextLogger::generate_signature(LogLevel level, std::uint16_t module_id)
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


PreciseTime get_precise_time()
{
	namespace chrono = std::chrono;
	auto chrono_time = chrono::system_clock::now();
	std::time_t seconds = chrono::system_clock::to_time_t(chrono_time);
	auto duration = chrono_time.time_since_epoch();
	using target_time_type = chrono::nanoseconds;
	auto nano = chrono::duration_cast<target_time_type>(duration).count() % target_time_type::period::den;
	return PreciseTime { seconds, nano };
}


BinaryLogger::BinaryLogger(std::uint16_t log_id, LogSinkPtr sink_ptr, StringTablePtr str_table_ptr) :
	m_sink_ptr(sink_ptr),
	m_str_table_ptr(str_table_ptr),
	m_writer(make_sequence(m_write_target), m_str_table_ptr)
{
	m_signature.set_log_id(log_id);
}


void BinaryLogger::log(LogLevel level,
					   std::uint16_t module_id,
					   const SourceLocation& location,
					   const char* str)
{
	if (this->should_log(level, module_id))
	{
		generate_signature(level, module_id, location, str);
		m_signature.set_argument_length(0);
		m_sink_ptr->write({&m_signature, m_signature.get_memory_size()});
	}
}


StringView BinaryLogReader::read()
{
	m_writer.clear();
	auto len = m_file.read(Sequence(&m_signature, m_signature.get_memory_size()));
	if (len != m_signature.get_memory_size())
	{
		return invalid_string_view();
	}

	std::unique_ptr<std::uint8_t[]> arguments(new std::uint8_t[m_signature.get_argument_length()]);
	m_file.read(Sequence(arguments.get(), m_signature.get_argument_length()));

	m_writer.write_text("[{}.{}] [{}] [{}.{}] ",
						Timestamp(m_signature.get_time().seconds),
						pad(m_signature.get_time().nanoseconds, '0', 10),
						to_string(m_signature.get_level()),
						m_signature.get_log_id(),
						m_signature.get_module_id());

	m_writer.write_binary(m_str_table_ptr->get_str(m_signature.get_description_id()).data(),
						  arguments.get(),
						  m_signature.get_argument_length());

	m_writer.write_text("  [{}:{}] [{}]",
						m_str_table_ptr->get_str(m_signature.get_file_id()),
						m_signature.get_line(),
						m_str_table_ptr->get_str(m_signature.get_function_id()));

	return m_writer.string_view();
}


void BinaryLogReader::jump(std::size_t line)
{
	for (std::size_t i = 0; i < line; ++i)
	{
		m_file.read({&m_signature, m_signature.get_memory_size()});
		auto pos = m_file.tell();
		m_file.seek(pos + m_signature.get_argument_length() + 1, FileSeekWhence::BEGIN);
	}
}

} // namespace lights
