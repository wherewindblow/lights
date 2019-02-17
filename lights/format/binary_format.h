/**
 * binary_format.h
 * @author wherewindblow
 * @date   Sep 01, 2017
 */

#pragma once

#include <cstdint>
#include <limits>

#include "../sequence.h"
#include "../format.h"
#include "../string_table.h"


namespace lights {

class BinaryStoreWriter;


/**
 * Enum all type of binary format support.
 */
enum class BinaryTypeCode: std::uint8_t
{
	INVALID = 0,
	BOOL = 1,
	CHAR = 2,
	STRING = 3,
	INT8_T = 4,
	UINT8_T = 5,
	INT16_T = 6,
	UINT16_T = 7,
	INT32_T = 8,
	UINT32_T = 9,
	INT64_T = 10,
	UINT64_T = 11,
	COMPOSED_TYPE  = 12,
	STRING_REF = 13,
	MAX
};


/**
 * BinaryStoreWriter use to store all format arguments and delay text format.
 * @note If the internal buffer is full will have no effect.
 */
class BinaryStoreWriter
{
public:
	enum FormatComposedTypeState
	{
		NO_INIT,
		STARTED,
		ENDED
	};

	/**
	 * Creates binary store writer.
	 * @param write_target If write target is not specify, will use default write target with default size.
	 */
	BinaryStoreWriter(Sequence write_target = invalid_sequence(), StringTable* str_table_ptr = nullptr);

	/**
	 * Copies binary store writer.
	 */
	BinaryStoreWriter(const BinaryStoreWriter& rhs);

	/**
	 * Destroys binary store writer.
	 */
	~BinaryStoreWriter();

	/**
	 * Copies binary store writer.
	 */
	BinaryStoreWriter& operator=(const BinaryStoreWriter& rhs);

	/**
	 * @note If the internal buffer is full will have no effect.
	 */
	void append(char ch);

	/**
	 * @note If the internal buffer is full will have no effect.
	 *       If @c store_in_table is set but string table is not set will also have no effect.
	 */
	void append(StringView str, bool store_in_table = false);

#define LIGHTSIMPL_BINARY_STORE_WRITER_INSERT_DECLARE(Type) \
	BinaryStoreWriter& operator<< (Type n);

	LIGHTSIMPL_ALL_INTEGER_FUNCTION(LIGHTSIMPL_BINARY_STORE_WRITER_INSERT_DECLARE)

#undef LIGHTSIMPL_BINARY_STORE_WRITER_INSERT_DECLARE

	/**
	 * It's only for write format to call and easy to restore.
	 * @note If the internal buffer is full will have no effect.
	 */
	template <typename T>
	void add_composed_type(const T& value);

	/**
	 * Forwards to @c lights::write() function.
	 * @note If the internal buffer is full will have no effect.
	 */
	template <typename Arg, typename ... Args>
	void write(StringView fmt, const Arg& value, const Args& ... args);

	/**
	 * Forwards to @c lights::write() function.
	 * @note If the internal buffer is full will have no effect.
	 */
	void write(StringView fmt);

	/**
	 * Returns the internal buffer.
	 */
	const std::uint8_t* data() const;

	/**
	 * Returns the internal buffer in c string but not null-terminated.
	 */
	const char* c_str() const;

	/**
	 * Returns a @c std::string that convert from internal buffer.
	 */
	std::string std_string() const;

	/**
	 * Returns a @c StringView of internal buffer.
	 * @note The return value is only valid when this object have no change
	 *       the area of return @c StringView.
	 */
	StringView string_view() const;

	/**
	 * Returns the length of internal buffer.
	 */
	std::size_t length() const;

	/**
	 * Returns the length of internal buffer.
	 * @details It's same as @c length() function.
	 */
	std::size_t size() const;

	/**
	 * Sets the size of internal buffer.
	 * @note Must ensure @c new_size is less than @c capacity().
	 */
	void resize(std::size_t new_size);

	/**
	 * Clears format result.
	 */
	void clear();

	/**
	 * Returns the max size that format result can be.
	 */
	std::size_t max_size() const;

	/**
	 * Returns the internal buffer size.
	 */
	std::size_t capacity() const;

private:
	/**
	 * Checks can append new content.
	 */
	bool can_append(std::size_t len);

	bool m_use_default_buffer;
	std::uint8_t* m_buffer;
	std::size_t m_length;
	std::size_t m_capacity;
	FormatComposedTypeState m_state;
	std::uint16_t m_composed_member_num;
	StringTable* m_str_table_ptr;
};


/**
 * FormatSink of BinaryStoreWriter.
 */
template <>
class FormatSink<BinaryStoreWriter>
{
public:
	/**
	 * Creates format sink.
	 */
	explicit FormatSink(BinaryStoreWriter& backend);

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

	/**
	 * Gets internal backend.
	 */
	BinaryStoreWriter& get_internal_backend();

private:
	BinaryStoreWriter& m_backend;
};


/**
 * Empty function.
 */
inline void write(FormatSink<BinaryStoreWriter> /* sink */, StringView /* fmt */)
{
}

/**
 * @note This explicit template specialization must declare before use it or cannot
 *       into this function and into general write function.
 */
template <typename Arg, typename ... Args>
void write(FormatSink<BinaryStoreWriter> sink, StringView fmt, const Arg& value, const Args& ... args)
{
	std::size_t i = 0;
	for (; i < fmt.length(); ++i)
	{
		if (fmt[i] == '{' &&
			i + 1 < fmt.length() &&
			fmt[i+1] == '}')
		{
			break;
		}
	}

	if (i < fmt.length())
	{
		sink.get_internal_backend().add_composed_type(value);
		fmt.move_forward(i + 2); // Skip "{}".
		write(sink, fmt, args ...);
	}
}


/**
 * Uses @c BinaryStoreWriter member function to format integer to speed up.
 */
#define LIGHTSIMPL_BINARY_STORE_WRITER_TO_STRING(Type) \
inline void to_string(FormatSink<BinaryStoreWriter> sink, Type n) \
{ \
	sink.get_internal_backend() << n; \
}

LIGHTSIMPL_ALL_INTEGER_FUNCTION(LIGHTSIMPL_BINARY_STORE_WRITER_TO_STRING)

#undef LIGHTSIMPL_BINARY_STORE_WRITER_TO_STRING


/**
 * BinaryRestoreWriter use to format text with arguments that store by @c BinaryStoreWriter.
 * @note If the internal buffer is full will have no effect.
 */
class BinaryRestoreWriter
{
public:
	BinaryRestoreWriter(String write_target = invalid_string(), StringTable* str_table_ptr = nullptr);

	/**
	 * @note If the internal buffer is full will have no effect.
	 */
	void append(char ch);

	/**
	 * @note If the internal buffer is full will have no effect.
	 */
	void append(StringView str);

	/**
	 * Forwards to lights::write() function to format text.
	 * @note If the internal buffer is full will have no effect.
	 */
	template <typename Arg, typename ... Args>
	void write_text(StringView fmt, const Arg& value, const Args& ... args);

	/**
	 * Forwards to lights::write() function to format text.
	 * @note If the internal buffer is full will have no effect.
	 */
	void write_text(StringView fmt);

	/**
	 * Forwards to lights::write() function to format binary.
	 * @note If the internal buffer is full will have no effect.
	 */
	void write_binary(StringView fmt, const std::uint8_t* binary_store_args, std::size_t args_length);

	/**
	 * Forwards to lights::operater<<() function.
	 * @return The reference of this object.
	 * @note If the internal buffer is full will have no effect.
	 */
	template <typename T>
	BinaryRestoreWriter& operator<< (const T& value);

	/**
	 * Returns a pointer to null-terminated character array that store in internal.
	 */
	const char* c_str() const;

	/**
	 * Returns a @c std::string that convert from internal buffer.
	 * @note This conversion will generate a data copy.
	 */
	std::string std_string() const;

	/**
	 * Returns a @c StringView of internal buffer.
	 * @note The return value is only valid when this object have no change
	 *       the area of return @c StringView.
	 */
	StringView string_view() const;

	/**
	 * Returns the length of internal buffer.
	 */
	std::size_t length() const;

	/**
	 * Returns the length of internal buffer.
	 * @details It's same as @c length() function.
	 */
	std::size_t size() const;

	/**
	 * Sets the format result length to zero.
	 */
	void clear();

	/**
	 * Returns the max size that format result can be.
	 */
	std::size_t max_size() const;

	/**
	 * Returns the internal buffer size.
	 */
	std::size_t capacity() const;

private:
	/**
	 * Writes a argument.
	 * @return Width of argument.
	 */
	std::uint8_t write_argument(const uint8_t* binary_store_args);

	TextWriter m_writer;
	StringTable* m_str_table_ptr;
};


// ========================= Implement. ==============================

template <typename Arg, typename ... Args>
inline void BinaryStoreWriter::write(StringView fmt, const Arg& value, const Args& ... args)
{
	lights::write(make_format_sink(*this), fmt, value, args ...);
}

/**
 * @note Must ensure the specialization of FormatSink is declare before use.
 */
inline void BinaryStoreWriter::write(StringView fmt)
{
	lights::write(make_format_sink(*this), fmt);
}

template <typename T>
void BinaryStoreWriter::add_composed_type(const T& value)
{
	if (m_state == FormatComposedTypeState::STARTED) // Reduce recursion.
	{
		make_format_sink(*this) << value;
	}
	else
	{
		m_composed_member_num = 0;
		std::uint8_t* type = m_buffer + m_length;
		auto composed_member_num = reinterpret_cast<std::uint16_t*>(m_buffer + m_length + sizeof(BinaryTypeCode));
		const auto composed_header_len = sizeof(BinaryTypeCode) + sizeof(std::uint16_t) / sizeof(std::uint8_t);
		m_length += composed_header_len;
		m_state = FormatComposedTypeState::STARTED;
		make_format_sink(*this) << value;
		m_state = FormatComposedTypeState::ENDED;

		if (m_composed_member_num > 1)
		{
			*type = static_cast<std::uint8_t>(BinaryTypeCode::COMPOSED_TYPE);
			*composed_member_num = m_composed_member_num;
		}
		else if (m_composed_member_num == 1)
		{
			std::size_t len = (m_buffer + m_length) - reinterpret_cast<std::uint8_t*>(composed_member_num + 1);
			std::memmove(type, composed_member_num + 1, len);
			m_length -= composed_header_len;
		}
		else // m_composed_member_num == 0, insert failure.
		{
			m_length -= composed_header_len;
		}
	}
}

inline const std::uint8_t* BinaryStoreWriter::data() const
{
	return m_buffer;
}

inline const char* BinaryStoreWriter::c_str() const
{
	return string_view().data();
}

inline std::string BinaryStoreWriter::std_string() const
{
	return string_view().to_std_string();
}

inline StringView BinaryStoreWriter::string_view() const
{
	return { reinterpret_cast<const char*>(m_buffer), m_length };
}

inline std::size_t BinaryStoreWriter::length() const
{
	return m_length;
}

inline std::size_t BinaryStoreWriter::size() const
{
	return m_length;
}

inline void BinaryStoreWriter::resize(std::size_t new_size)
{
	m_length = new_size;
}

inline void BinaryStoreWriter::clear()
{
	m_length = 0;
}

inline std::size_t BinaryStoreWriter::max_size() const
{
	return m_capacity;
}

inline std::size_t BinaryStoreWriter::capacity() const
{
	return m_capacity;
}

inline bool BinaryStoreWriter::can_append(std::size_t len)
{
	return m_length + len <= max_size();
}


inline FormatSink<BinaryStoreWriter>::FormatSink(BinaryStoreWriter& backend) :
	m_backend(backend)
{}

inline void FormatSink<BinaryStoreWriter>::append(char ch)
{
	m_backend.append(ch);
}

inline void FormatSink<BinaryStoreWriter>::append(StringView str)
{
	m_backend.append(str);
}

inline BinaryStoreWriter& FormatSink<BinaryStoreWriter>::get_internal_backend()
{
	return m_backend;
}


inline BinaryRestoreWriter::BinaryRestoreWriter(String write_target, StringTable* str_table_ptr) :
	m_writer(write_target),
	m_str_table_ptr(str_table_ptr)
{}

inline void BinaryRestoreWriter::append(char ch)
{
	m_writer.append(ch);
}

inline void BinaryRestoreWriter::append(StringView str)
{
	m_writer.append(str);
}

template <typename Arg, typename... Args>
inline void BinaryRestoreWriter::write_text(StringView fmt, const Arg& value, const Args& ... args)
{
	// Must add namespace scope limit or cannot find suitable function.
	lights::write(make_format_sink(m_writer), fmt, value, args ...);
}

inline void BinaryRestoreWriter::write_text(StringView fmt)
{
	lights::write(make_format_sink(m_writer), fmt);
}

template <typename T>
inline BinaryRestoreWriter& BinaryRestoreWriter::operator<<(const T& value)
{
	make_format_sink(m_writer) << value;
	return *this;
}

inline const char* BinaryRestoreWriter::c_str() const
{
	return m_writer.c_str();
}

inline std::string BinaryRestoreWriter::std_string() const
{
	return m_writer.std_string();
}

inline StringView BinaryRestoreWriter::string_view() const
{
	return m_writer.string_view();
}

inline std::size_t BinaryRestoreWriter::length() const
{
	return m_writer.length();
}

inline std::size_t BinaryRestoreWriter::size() const
{
	return m_writer.size();
}

inline void BinaryRestoreWriter::clear()
{
	m_writer.clear();
}

inline std::size_t BinaryRestoreWriter::max_size() const
{
	return m_writer.max_size();
}

inline std::size_t BinaryRestoreWriter::capacity() const
{
	return m_writer.capacity();
}

} // namespace lights
