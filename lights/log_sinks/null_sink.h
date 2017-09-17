/**
 * null_sink.h
 * @author wherewindblow
 * @date   Dec 18, 2016
 */

#pragma once

#include <cstddef>
#include <memory>

#include "../sequence.h"


namespace lights {
namespace log_sinks {

class NullSink: lights::NullSink
{
public:
	static std::shared_ptr<NullSink> instance()
	{
		static auto instance = std::make_shared<NullSink>();
		return instance;
	};
};

} // namespace log_sinks
} // namespace lights
