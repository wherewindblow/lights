/**
 * sink.cpp
 * @author wherewindblow
 * @date   Feb 16, 2019
 */

#include "sink.h"


namespace lights {

void FormatSink<Sink>::append(std::size_t num, char ch)
{
	for (std::size_t i = 0; i < num; ++i)
	{
		this->append(ch);
	}
}

} // namespace lights
