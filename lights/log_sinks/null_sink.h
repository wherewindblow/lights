/**
 * null_sink.h
 * @author wherewindblow
 * @date   Dec 18, 2016
 */

#pragma once

#include "../sequence.h"


namespace lights {
namespace log_sinks {

/**
 * NullSink will accept all data and do nothing.
 */
class NullSink: lights::NullSink
{
public:
	/**
	 * Returns instance.
	 */
	static NullSink& instance()
	{
		static NullSink inst;
		return inst;
	};
};

} // namespace log_sinks
} // namespace lights
