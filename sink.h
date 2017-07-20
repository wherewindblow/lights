/**
 * sink.h
 * @author wherewindblow
 * @date   Jul 20, 2017
 */

#pragma once

#include <cstddef>


namespace lights {

class SinkAdapter
{
public:
	SinkAdapter() = default;
	virtual ~SinkAdapter() = default;

	virtual std::size_t write(const void* buf, std::size_t len) = 0;
};


class NullSinkAdapter: public SinkAdapter
{
public:
	std::size_t write(const void* buf, std::size_t len) override
	{
		return len;
	};
};

} // namespace lights
