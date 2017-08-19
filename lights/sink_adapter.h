/**
 * sink_adapter.h
 * @author wherewindblow
 * @date   Jul 20, 2017
 */

#pragma once

#include <cstddef>

#include "sequence.h"


namespace lights {

class SinkAdapter
{
public:
	SinkAdapter() = default;
	virtual ~SinkAdapter() = default;

	virtual std::size_t write(SequenceView buffer) = 0;
};


class NullSinkAdapter: public SinkAdapter
{
public:
	std::size_t write(SequenceView buffer) override
	{
		return buffer.length();
	};
};


inline SinkAdapter& operator<< (SinkAdapter& sink, SequenceView sequence)
{
	sink.write(sequence);
	return sink;
}

inline SinkAdapter& operator<< (SinkAdapter& sink, StringView str)
{
	sink.write(str);
	return sink;
}


template <typename Sink>
class FormatSinkAdapter;

template <>
class FormatSinkAdapter<SinkAdapter>
{
public:
	explicit FormatSinkAdapter(SinkAdapter& sink) : m_sink(sink) {}

	void append(char ch)
	{
		m_sink.write(SequenceView(&ch, sizeof(ch)));
	}

	void append(std::size_t num, char ch)
	{
		for (std::size_t i = 0; i < num; ++i)
		{
			this->append(ch);
		}
	}

	void append(StringView str)
	{
		m_sink.write(str);
	}

private:
	SinkAdapter& m_sink;
};

} // namespace lights
