/**
 * sink_adapter.h
 * @author wherewindblow
 * @date   Jul 20, 2017
 */

#pragma once

#include <cstddef>

#include "sequence.h"


namespace lights {

/**
 * SinkAdapter is a virtual base class and use as backend of ouput.
 */
class SinkAdapter
{
public:
	SinkAdapter() = default;
	virtual ~SinkAdapter() = default;

	virtual std::size_t write(SequenceView buffer) = 0;
};

/**
 * NullSinkAdapter will accept all data and do nothing.
 */
class NullSinkAdapter: public SinkAdapter
{
public:
	std::size_t write(SequenceView buffer) override
	{
		return buffer.length();
	};
};


/**
 * Inserts @c SequenceView object to @c SinkAdapter object.
 */
inline SinkAdapter& operator<< (SinkAdapter& sink, SequenceView sequence)
{
	sink.write(sequence);
	return sink;
}

/**
 * Inserts @c StringView object to @c SinkAdapter object.
 */
inline SinkAdapter& operator<< (SinkAdapter& sink, StringView str)
{
	sink.write(str);
	return sink;
}


template <typename Sink>
class FormatSinkAdapter;

/**
 * Explicit template specialization for SinkAdapter.
 * Aim to allow change sink in runtime via inherit from SinkAdapter.
 */
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
