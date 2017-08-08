/**
 * sink_adapter.h
 * @author wherewindblow
 * @date   Jul 20, 2017
 */

#pragma once

#include <cstddef>

#include "block_description.h"


namespace lights {

class SinkAdapter
{
public:
	SinkAdapter() = default;
	virtual ~SinkAdapter() = default;

	virtual std::size_t write(BufferView buffer) = 0;
};


class NullSinkAdapter: public SinkAdapter
{
public:
	std::size_t write(BufferView buffer) override
	{
		return buffer.length();
	};
};


inline SinkAdapter& operator<< (SinkAdapter& sink, BufferView buffer)
{
	sink.write(buffer);
	return sink;
}

inline SinkAdapter& operator<< (SinkAdapter& sink, StringView str)
{
	sink.write(str);
	return sink;
}

} // namespace lights
