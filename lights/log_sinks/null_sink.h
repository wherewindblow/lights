/**
 * null_sink.h
 * @author wherewindblow
 * @date   Dec 18, 2016
 */

#pragma once

#include <cstddef>
#include <memory>

#include "lights/block_description.h"


namespace lights {
namespace log_sinks {

class NullSink
{
public:
	void write(BufferView buffer)
	{}

	static std::shared_ptr<NullSink> instance()
	{
		static auto instance = std::make_shared<NullSink>();
		return instance;
	};
};

} // namespace log_sinks
} // namespace lights
