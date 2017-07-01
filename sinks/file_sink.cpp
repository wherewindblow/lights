/*
 * file_sink.cpp
 * @author wherewindblow
 * @date   May 11, 2017
 */

#include "file_sink.h"


namespace lights {

#define LIGHTS_LOGGER_FILE_SINK_NO_INLINE(Sink) \
TextLogger<Sink>::TextLogger(StringView name, std::shared_ptr<Sink> sink) : \
	m_name(name.data), m_sink(sink), m_writer(std::make_unique<TextWriter<BUFSIZ>>()) \
{ \
	m_writer->set_full_handler([&](StringView view){ \
		m_sink->write(view.data, view.length); \
	}); \
} \
\
TextLogger<Sink>::~TextLogger() \
{ \
	StringView view = m_writer->str_view(); \
	m_sink->write(view.data, view.length); \
} \
\
\
void TextLogger<Sink>::generate_signature() \
{ \
	namespace chrono = std::chrono; \
	auto chrono_time = chrono::system_clock::now(); \
	std::time_t time = chrono::system_clock::to_time_t(chrono_time); \
 \
	*m_writer << '[' << Timestamp(time) << '.'; \
 \
	auto duration = chrono_time.time_since_epoch(); \
	auto millis = chrono::duration_cast<chrono::milliseconds>(duration).count() % 1000; \
	*m_writer << pad(static_cast<unsigned>(millis), '0', 3); \
 \
	*m_writer << "] [" << m_name << "] [" << to_string(m_level) << "] "; \
}

LIGHTS_LOGGER_FILE_SINK_NO_INLINE(sinks::SimpleFileSink)

} // namespace lights

