/**
 * null_sink.h
 * @author wherewindblow
 * @date   Dec 18, 2016
 */

#pragma once

#include <cstddef>
#include <memory>


namespace lights {
namespace sinks {

class NullSink
{
public:
	void write(const char* str, std::size_t len)
	{}

	static std::shared_ptr<NullSink> get_instance()
	{
		static auto instance = std::make_shared<NullSink>();
		return instance;
	};
};

} // namespace sinks
} // namespace lights
