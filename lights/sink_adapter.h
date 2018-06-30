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
 * SinkAdapter is virtual base class and use as backend of ouput.
 */
class SinkAdapter
{
public:
	SinkAdapter() = default;
	virtual ~SinkAdapter() = default;

	/**
	 * Writes sequence to backend.
	 * @note It's pure virtual function and must implementaion by derived class.
	 */
	virtual std::size_t write(SequenceView sequence) = 0;
};


/**
 * NullSink will accept all data and do nothing.
 */
class NullSink: public SinkAdapter
{
public:
	/**
	 * Writes sequence and do nothing.
	 */
	std::size_t write(SequenceView sequence) override
	{
		return sequence.length();
	};
};


/**
 * Inserts sequence into sink.
 */
inline SinkAdapter& operator<< (SinkAdapter& sink, SequenceView sequence)
{
	sink.write(sequence);
	return sink;
}


/**
 * Inserts string into sink.
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
	/**
	 * Creates format sink.
	 */
	explicit FormatSinkAdapter(SinkAdapter& sink) : m_sink(sink) {}

	/**
	 * Appends char to backend.
	 */
	void append(char ch)
	{
		m_sink.write(SequenceView(&ch, sizeof(ch)));
	}

	/**
	 * Appends multiple same char to backend.
	 */
	void append(std::size_t num, char ch)
	{
		for (std::size_t i = 0; i < num; ++i)
		{
			this->append(ch);
		}
	}

	/**
	 * Appends string to backend.
	 */
	void append(StringView str)
	{
		m_sink.write(str);
	}

private:
	SinkAdapter& m_sink;
};

} // namespace lights
