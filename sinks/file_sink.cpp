/*
 * file_sink.cpp
 * @author wherewindblow
 * @date   May 11, 2017
 */

#include "file_sink.h"


namespace lights {

#define LIGHTS_LOGGER_FILE_SINK_NO_INLINE(Sink) \
Logger<Sink>::Logger(const std::string& name, std::shared_ptr<Sink> sink) : \
	m_name(name), m_sink(sink), m_writer(std::make_unique<MemoryWriter<BUFSIZ>>()) \
{ \
	m_writer->set_full_handler([&](StringView view){ \
		m_sink->write(view.string, view.length); \
	}); \
} \
\
Logger<Sink>::~Logger() \
{ \
	StringView view = m_writer->str_view(); \
	m_sink->write(view.string, view.length); \
} \
\
\
void Logger<Sink>::generate_signature_header() \
{ \
	namespace chrono = std::chrono; \
	auto chrono_time = chrono::system_clock::now(); \
	std::time_t time = chrono::system_clock::to_time_t(chrono_time); \
	std::tm tm; \
	localtime_r(&time, &tm); \
 \
	*m_writer << '['; \
 \
	*m_writer << static_cast<unsigned>(tm.tm_year + 1900) << '-'; \
	write_2_digit(*m_writer, static_cast<unsigned>(tm.tm_mon + 1)) << '-'; \
	write_2_digit(*m_writer, static_cast<unsigned>(tm.tm_mday)) << ' '; \
	write_2_digit(*m_writer, static_cast<unsigned>(tm.tm_hour)) << ':'; \
	write_2_digit(*m_writer, static_cast<unsigned>(tm.tm_min)) << ':'; \
	write_2_digit(*m_writer, static_cast<unsigned>(tm.tm_sec)) << '.'; \
 \
	auto duration = chrono_time.time_since_epoch(); \
	auto millis = chrono::duration_cast<chrono::milliseconds>(duration).count() % 1000; \
	*m_writer << pad(static_cast<unsigned>(millis), '0', 3); \
 \
	*m_writer << "] [" << m_name << "] [" << to_string(m_level) << "] "; \
}

LIGHTS_LOGGER_FILE_SINK_NO_INLINE(sinks::SimpleFileSink)

} // namespace lights

