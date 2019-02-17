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
 * Sink is virtual base class and use as backend of output.
 */
class Sink
{
public:
	Sink() = default;
	virtual ~Sink() = default;

	/**
	 * Writes sequence to backend.
	 * @note It's pure virtual function and must implementation by derived class.
	 */
	virtual std::size_t write(SequenceView sequence) = 0;
};


/**
 * NullSink will accept all data and do nothing.
 */
class NullSink: public Sink
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
inline Sink& operator<< (Sink& sink, SequenceView sequence)
{
	sink.write(sequence);
	return sink;
}


/**
 * Inserts string into sink.
 */
inline Sink& operator<< (Sink& sink, StringView str)
{
	sink.write(str);
	return sink;
}


template <typename Backend>
class FormatSink;

/**
 * Explicit template specialization for Sink.
 * Aim to allow change sink in runtime via inherit from Sink.
 */
template <>
class FormatSink<Sink>
{
public:
	/**
	 * Creates format sink.
	 */
	explicit FormatSink(Sink& sink);

	/**
	 * Appends char to backend.
	 */
	void append(char ch);

	/**
	 * Appends multiple same char to backend.
	 */
	void append(std::size_t num, char ch);

	/**
	 * Appends string to backend.
	 */
	void append(StringView str);

private:
	Sink& m_backend;
};


// ============================== Implement. ===============================

inline FormatSink<Sink>::FormatSink(Sink& sink) :
	m_backend(sink)
{}

inline void FormatSink<Sink>::append(char ch)
{
	m_backend.write(SequenceView(&ch, sizeof(ch)));
}

inline void FormatSink<Sink>::append(StringView str)
{
	m_backend.write(str);
}

} // namespace lights
