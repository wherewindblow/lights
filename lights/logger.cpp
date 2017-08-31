/**
 * logger.cpp
 * @author wherewindblow
 * @date   Jul 23, 2017
 */

#include "logger.h"

#include <chrono>


namespace lights {

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


StringView BinaryLogReader::read()
{
	m_writer.clear();
	auto len = m_file.read(Sequence(&m_signature, m_signature.get_memory_size()));
	if (len != m_signature.get_memory_size())
	{
		return StringView(nullptr, 0);
	}

	std::unique_ptr<std::uint8_t[]> arguments(new std::uint8_t[m_signature.get_argument_length()]);
	m_file.read(Sequence(arguments.get(), m_signature.get_argument_length()));

	m_writer.write_text("[{}.{}] [{}] [{}.{}] ",
						Timestamp(m_signature.get_time().seconds),
						pad(m_signature.get_time().nanoseconds, '0', 10),
						to_string(m_signature.get_level()),
						m_signature.get_log_id(),
						m_signature.get_module_id());

	m_writer.write_binary(m_str_table->get_str(m_signature.get_description_id()).data(),
						  arguments.get(),
						  m_signature.get_argument_length());

	m_writer.write_text("  [{}:{}] [{}]",
						m_str_table->get_str(m_signature.get_file_id()),
						m_signature.get_line(),
						m_str_table->get_str(m_signature.get_function_id()));

	return m_writer.str_view();
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
