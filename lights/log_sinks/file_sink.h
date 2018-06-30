/**
 * file_sink.h
 * @author wherewindblow
 * @date   Dec 18, 2016
 */

#pragma once

#include <cstddef>
#include <cstring>
#include <string>
#include <memory>
#include <mutex>

#include "../sequence.h"
#include "../file.h"
#include "../exception.h"


namespace lights {
namespace log_sinks {

/**
 * LogMessageWriter ensure every log message is write to backend completely.
 * And support difference policy to flush log message to backend.
 */
class LogMessageWriter
{
public:
	/**
	 * Creates writer.
	 */
	LogMessageWriter(FileStream* file = nullptr) :
		m_file(file) {}

	/**
	 * Sets log message write target.
	 */
	void set_write_target(FileStream* file)
	{
		m_file = file;
		m_buffer_length = 0;
	}

	/**
	 * Writes log message into backend.
	 */
	std::size_t write(SequenceView log_msg)
	{
		if (m_buffer_length + log_msg.length() > FILE_DEFAULT_BUFFER_SIZE)
		{
			m_file->flush();
			m_buffer_length = 0;
		}

		return m_file->write(log_msg);
	}

private:
	std::size_t m_buffer_length = 0;
	FileStream* m_file = nullptr;
};


/**
 * SimpleFileSink write all log message into one file.
 */
class SimpleFileSink: public Sink
{
public:
	/**
	 * Creates sink.
	 */
	SimpleFileSink(StringView filename) :
		m_file(filename.data(), "ab+"), m_msg_writer(&m_file) {}

	/**
	 * Writes log message into backend.
	 * @details Write is atomic.
	 */
	std::size_t write(SequenceView log_msg) override
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_msg_writer.write(log_msg);
	}

private:
	FileStream m_file;
	LogMessageWriter m_msg_writer;
	std::mutex m_mutex;
};


/**
 * SizeRotatingFileSink write log message into file. If file is achieve size limit will rotate to next file.
 */
class SizeRotatingFileSink: public Sink
{
public:
	SizeRotatingFileSink() = default;

#define LIGHTS_SINKS_INIT_MEMBER(exp) \
	if (can_init) \
	{             \
		exp;      \
	}             \
	else          \
	{             \
		LIGHTS_ASSERT(false && "SizeRotatingFileSink: Cannot initialize when have been end initialization"); \
	}

	/**
	 * Init file name format.
	 * @param name_format  Format string that use "{}" as a placehalder.
	 * @note This init is necessary.
	 */
	void init_name_format(const std::string& name_format)
	{
		LIGHTS_SINKS_INIT_MEMBER(m_name_format = name_format);
	}

	/**
	 * @note If not init max size, the biggest of std::size_t value is use.
	 */
	void init_max_size(std::size_t max_size)
	{
		LIGHTS_SINKS_INIT_MEMBER(m_max_size = max_size);
	}

	/**
	 * @note If not int max files, it'll not cycle to use file name.
	 */
	void init_max_files(std::size_t max_files)
	{
		LIGHTS_SINKS_INIT_MEMBER(m_max_files = max_files);
	}

#undef LIGHTS_SINKS_INIT_MEMBER

	/**
	 * Ends initialization.
	 */
	void end_init()
	{
		can_init = false;
		this->rotate(0);
	}

	/**
	 * Writes log message into backend.
	 * @details Write is atomic.
	 */
	std::size_t write(SequenceView log_msg) override
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		while (m_current_size + log_msg.length() > m_max_size)
		{
			this->rotate(log_msg.length());
		}
		std::size_t writed_length = m_msg_writer.write(log_msg);
		m_current_size += writed_length;
		return writed_length;
	}

private:
	void fill_remain();

	void rotate(std::size_t expect_size);

	bool can_init = true;
	std::string m_name_format;
	std::size_t m_max_size;
	std::size_t m_max_files = static_cast<std::size_t>(-1);
	FileStream m_file;
	LogMessageWriter m_msg_writer;
	std::size_t m_index = static_cast<std::size_t>(-1);
	std::size_t m_current_size;
	std::mutex m_mutex;
};


/**
 * TimeRotatingFileSink write log message into file. If time is achieve limit will rotate to next file.
 */
class TimeRotatingFileSink: public Sink
{
public:
	static constexpr std::time_t ONE_DAY_SECONDS = 3600 * 24;

	/**
	 * @param name_format  Must have a placeholder "{}" and it'll replace by rotating time.
	 * @param duration     Within a duration, all message will log to same file.
	 * @param day_point    The seconds of a day that will execute rotating.
	 */
	TimeRotatingFileSink(std::string name_format,
						 std::time_t duration = ONE_DAY_SECONDS,
						 std::time_t day_point = 0);

	/**
	 * Writes log message into backend.
	 * @details Write is atomic.
	 */
	std::size_t write(SequenceView log_msg) override
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		std::time_t now = std::time(nullptr);
		if (now >= m_next_rotating_time)
		{
			rotate();
		}
		return m_msg_writer.write(log_msg);
	}

private:
	void rotate();

	std::string m_name_format;
	std::time_t m_duration;
	std::time_t m_day_point;
	std::time_t m_next_rotating_time;
	FileStream m_file;
	LogMessageWriter m_msg_writer;
	std::mutex m_mutex;
};

} // namespace log_sinks
} // namespace lights
