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


void TextLogger::log(LogLevel level, const SourceLocation& location, const char* str)
{
	if (this->should_log(level))
	{
		m_writer.clear();
		this->generate_signature(level);
		m_writer.append(str);
		this->recore_location(location);
		append_log_seperator();
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


void TextLogger::generate_signature(LogLevel level)
{
	PreciseTime precise_time = current_precise_time();
	m_writer << '[' << Timestamp(precise_time.seconds) << '.';

	auto millis = precise_time.nanoseconds / 1000 / 1000;
	m_writer << pad(static_cast<unsigned>(millis), '0', 3);
	m_writer << "] [" << m_name << "] ";
	m_writer << "[" << to_string(level) << "] ";
}


void TextLogger::append_log_seperator()
{
	StringView end_line = env::end_line();
	m_writer.append(end_line);
	if (m_writer.size() == m_writer.max_size())
	{
		char* last_char = &m_write_target[m_writer.size() - end_line.length()];
		if (last_char != end_line)
		{
			copy_array(last_char, end_line.data(), end_line.length());
		}
	}
}


PreciseTime current_precise_time()
{
	namespace chrono = std::chrono;
	auto chrono_time = chrono::system_clock::now();
	std::time_t seconds = chrono::system_clock::to_time_t(chrono_time);
	auto duration = chrono_time.time_since_epoch();
	using target_time_type = chrono::nanoseconds;
	auto nano = chrono::duration_cast<target_time_type>(duration).count() % target_time_type::period::den;
	return PreciseTime { seconds, nano };
}


BinaryLogger::BinaryLogger(std::uint16_t logger_id, LogSinkPtr sink_ptr, StringTablePtr str_table_ptr) :
	m_sink_ptr(sink_ptr),
	m_str_table_ptr(str_table_ptr),
	m_signature(reinterpret_cast<BinaryMessageSignature*>(m_write_target)),
	m_writer(Sequence(m_write_target + sizeof(BinaryMessageSignature),
					  sizeof(m_write_target) - sizeof(BinaryMessageSignature) - sizeof(std::uint16_t)),
		// sizeof(std::uint16_t) is reverse for tail length.
			 m_str_table_ptr)
{
	m_signature->logger_id = logger_id;
}


void BinaryLogger::generate_signature(LogLevel level, const SourceLocation& location, StringView description)
{
	auto time = current_precise_time();
	m_signature->time_seconds = time.seconds;
	m_signature->time_nanoseconds = time.nanoseconds;
	auto file_id = m_str_table_ptr->get_index(location.file());
	m_signature->file_id = static_cast<std::uint32_t>(file_id);
	auto function_id = m_str_table_ptr->get_index(location.function());
	m_signature->function_id = static_cast<std::uint32_t>(function_id);
	m_signature->source_line = location.line();
	m_signature->description_id = static_cast<std::uint32_t>(m_str_table_ptr->get_index(description));
	m_signature->level = level;
}


void BinaryLogger::log(LogLevel level, const SourceLocation& location, const char* str)
{
	if (this->should_log(level))
	{
		this->generate_signature(level, location, str);
		set_argument_length(0);
		this->sink_msg();
	}
}


BinaryLogReader::BinaryLogReader(StringView log_filename, StringTablePtr str_table_ptr) :
	m_file(log_filename, "rb"),
	m_str_table_ptr(str_table_ptr),
	m_writer(make_string(m_write_target), str_table_ptr)
{
}


StringView BinaryLogReader::read()
{
	m_writer.clear();
	auto len = m_file.read(Sequence(&m_signature, sizeof(BinaryMessageSignature)));
	if (len != sizeof(BinaryMessageSignature))
	{
		return invalid_string_view();
	}

	std::unique_ptr<std::uint8_t[]> arguments(new std::uint8_t[m_signature.argument_length]);
	m_file.read(Sequence(arguments.get(), m_signature.argument_length));
	std::uint16_t tail_length;
	m_file.read(Sequence(&tail_length, sizeof(tail_length)));

	m_writer.write_text("[{}.{}] [{}] [{}] ",
						Timestamp(m_signature.time_seconds),
						pad(m_signature.time_nanoseconds, '0', 10),
						to_string(m_signature.level),
						m_signature.logger_id);

	m_writer.write_binary(m_str_table_ptr->get_str(m_signature.description_id).data(),
						  arguments.get(),
						  m_signature.argument_length);

	m_writer.write_text("  [{}:{}] [{}]",
						m_str_table_ptr->get_str(m_signature.file_id),
						m_signature.source_line,
						m_str_table_ptr->get_str(m_signature.function_id));

	return m_writer.string_view();
}


void BinaryLogReader::jump(std::streamoff line)
{
	if (line == 0)
	{
		return;
	}
	else if (line > 0)
	{
		this->jump_from_head(static_cast<std::size_t>(line));
	}
	else
	{
		this->jump_from_tail(static_cast<std::size_t>(0 - line));
	}
}


void BinaryLogReader::jump_from_head(std::size_t line)
{
	for (std::size_t i = 0; i < line; ++i)
	{
		m_file.read({&m_signature, sizeof(BinaryMessageSignature)});
		auto pos = m_file.tell();
		pos += m_signature.argument_length + sizeof(std::uint16_t);
		m_file.seek(pos, FileSeekWhence::BEGIN);
	}
}


void BinaryLogReader::jump_from_tail(std::size_t line)
{
	m_file.seek(0, FileSeekWhence::END);
	for (std::size_t i = 0; i < line; ++i)
	{
		std::uint16_t tail_length;
		std::streamoff tail_length_pos = m_file.tell() - sizeof(tail_length);
		m_file.seek(tail_length_pos, FileSeekWhence::BEGIN);
		m_file.read(Sequence(&tail_length, sizeof(tail_length)));
		std::streamoff previous_pos = m_file.tell() - (sizeof(BinaryMessageSignature) + tail_length + sizeof(tail_length));
		m_file.seek(previous_pos, FileSeekWhence::BEGIN);
	}
}

} // namespace lights
