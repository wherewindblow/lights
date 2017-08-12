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

} // namespace lights
