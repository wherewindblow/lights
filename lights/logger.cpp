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


StringTable::StringTable(StringView filename)
{
	m_file.open(filename.data());
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
		m_file.open(filename.data(), std::ios_base::out); // Create file.
		if (!m_file.is_open())
		{
			LIGHTS_THROW_EXCEPTION(OpenFileError, filename);
		}
	}
}


StringTable::~StringTable()
{
	if (m_file.is_open())
	{
		m_file.seekp(0, std::ios_base::end);
		m_file.clear();
		for (std::size_t i = m_last_index + 1; m_file && i < m_str_array.size(); ++i)
		{
			m_file.write(m_str_array[i]->data(), m_str_array[i]->length());
			m_file << LIGHTS_LINE_ENDER;
		}
		m_file.close();
	}
}


std::size_t StringTable::get_str_index(StringView str)
{
	StringViewPtr str_ptr(&str, EmptyDeleter());
	auto itr = m_str_hash.find(str_ptr);
	if (itr == m_str_hash.end())
	{
		return add_str(str);
	}
	else
	{
		return itr->second;
	}
}


std::size_t StringTable::add_str(StringView str)
{
	char* storage = new char[str.length()];
	std::memcpy(storage, str.data(), str.length());
	StringView* new_view = new StringView(storage, str.length());
	StringViewPtr str_ptr(new_view, StringDeleter());

	m_str_array.push_back(str_ptr);
	auto pair = std::make_pair(str_ptr, m_str_array.size() - 1);
	m_str_hash.insert(pair);
	return pair.second;
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
