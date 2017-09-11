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

template <std::size_t buffer_size>
class BinaryStoreWriter;

/**
 * Empty function.
 */
template <std::size_t buffer_size>
inline void write(FormatSinkAdapter<BinaryStoreWriter<buffer_size>> /* out */, StringView /* fmt */)
{
}

/**
 * @note This explicit template specialization must declare before use it or cannot
 *       into this function and into general write function.
 */
template <std::size_t buffer_size, typename Arg, typename ... Args>
void write(FormatSinkAdapter<BinaryStoreWriter<buffer_size>> out,
		   StringView fmt, const Arg& value, const Args& ... args)
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
		out.get_internal_sink().add_composed_type(value);
		fmt.move_forward(i + 2);
		write(out, fmt, args ...);
	}
}


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


inline BinaryTypeCode get_type_code(bool)
{
	return BinaryTypeCode::BOOL;
}

inline BinaryTypeCode get_type_code(char)
{
	return BinaryTypeCode::CHAR;
}

inline BinaryTypeCode get_type_code(const char*)
{
	return BinaryTypeCode::STRING;
}

/**
 * Get @c BinaryTypeCode by integer type @c T.
 */
template <typename T>
inline BinaryTypeCode get_type_code(T)
{
	int offset;
	switch (std::numeric_limits<std::make_unsigned_t<T>>::digits)
	{
		case 8:
			offset = 0;
			break;
		case 16:
			offset = 1;
			break;
		case 32:
			offset = 2;
			break;
		case 64:
			offset = 3;
			break;
		default:
			offset = 0;
			break;
	}

	offset *= 2;
	offset += !std::numeric_limits<T>::is_signed ? 1 : 0;
	return static_cast<BinaryTypeCode>(static_cast<int>(BinaryTypeCode::INT8_T) + offset);
}

/**
 * Get type with of type by BinaryTypeCode.
 */
inline std::uint8_t get_type_width(BinaryTypeCode code)
{
	static std::uint8_t widths[] = {
		0, // Invalid.
		1, 1, // Bool and char
		1,    // String.
		1, 1, // 8 bits
		2, 2, // 16 bits
		4, 4, // 32 bits
		8, 8, // 64 bits
		2,    // User-define composed type
		4,    // String reference only store a string table index.
	};

	std::uint8_t index = static_cast<std::uint8_t>(code);
	if (index < 0 || index >= static_cast<std::uint8_t>(BinaryTypeCode::MAX))
	{
		return widths[0];
	}
	else
	{
		return widths[index];
	}
}

/**
 * BinaryStoreWriter use to store all format arguments and delay text format.
 * @tparam buffer_size  Default size is @c WRITER_BUFFER_SIZE_DEFAULT
 * @note If the internal buffer is full will have no effect.
 */
template <std::size_t buffer_size = WRITER_BUFFER_SIZE_DEFAULT>
class BinaryStoreWriter
{
public:
	enum FormatComposedTypeState
	{
		NO_INIT,
		STARTED,
		ENDED
	};

	BinaryStoreWriter() = default;

	BinaryStoreWriter(StringTablePtr str_table):
		m_str_table(str_table) {}

	/**
	 * @note If the internal buffer is full will have no effect.
	 */
	void append(char ch)
	{
		if (can_append(sizeof(BinaryTypeCode) + sizeof(ch)))
		{
			if (m_state == FormatComposedTypeState::STARTED)
			{
				++m_composed_member_num;
			}
			m_buffer[m_length++] = static_cast<std::uint8_t>(BinaryTypeCode::CHAR);
			m_buffer[m_length++] = static_cast<std::uint8_t>(ch);
		}
	}

	/**
	 * @note If the internal buffer is full will have no effect.
	 *       If @c store_in_table is set but string table is not set will also have no effect.
	 */
	void append(StringView str, bool store_in_table = false)
	{
		if (str.length() == 0)
		{
			return;
		}
		else if (str.length() == 1)
		{
			append(str[0]);
		}
		else
		{
			if (store_in_table && m_str_table)
			{
				BinaryTypeCode type_code = BinaryTypeCode::STRING_REF;
				if (can_append(sizeof(BinaryTypeCode) + get_type_width(type_code)))
				{
					if (m_state == FormatComposedTypeState::STARTED)
					{
						++m_composed_member_num;
					}
					m_buffer[m_length++] = static_cast<std::uint8_t>(type_code);
					std::uint32_t* p = reinterpret_cast<std::uint32_t *>(&m_buffer[m_length]);
					auto index = static_cast<std::uint32_t>(m_str_table->get_index(str));
					*p = index;
					m_length += get_type_width(type_code);
				}
			}
			else if (can_append(str.length() + sizeof(BinaryTypeCode) + sizeof(std::uint8_t)))
			{
				if (m_state == FormatComposedTypeState::STARTED)
				{
					++m_composed_member_num;
				}
				m_buffer[m_length++] = static_cast<std::uint8_t>(BinaryTypeCode::STRING);
				m_buffer[m_length++] = static_cast<std::uint8_t>(str.length());
				std::memcpy(m_buffer + m_length, str.data(), str.length());
				m_length += str.length();
			}
		}
	}

#define LIGHTS_BINARY_STORE_WRITER_APPEND_INTEGER_BODY(Type) \
	{ \
		BinaryTypeCode type_code = get_type_code(n); \
		if (can_append(sizeof(BinaryTypeCode) + get_type_width(type_code))) \
		{ \
			if (m_state == FormatComposedTypeState::STARTED) \
			{ \
				++m_composed_member_num;\
			} \
			m_buffer[m_length++] = static_cast<std::uint8_t>(type_code); \
			Type* p = reinterpret_cast<Type *>(&m_buffer[m_length]); \
			*p = n; \
			m_length += get_type_width(type_code); \
		} \
		return *this; \
	}

	BinaryStoreWriter& operator<< (std::int8_t n) \
	{
		LIGHTS_BINARY_STORE_WRITER_APPEND_INTEGER_BODY(std::int8_t);
	}

	BinaryStoreWriter& operator<< (std::uint8_t n) \
	{
		LIGHTS_BINARY_STORE_WRITER_APPEND_INTEGER_BODY(std::uint8_t);
	}

	/**
	 * Inserts a integer and convert to small integer type when can convert.
	 */
#define LIGHTS_BINARY_STORE_WRITER_APPEND_SIGNED_INTEGER(Type, FitSmallType) \
	BinaryStoreWriter& operator<< (Type n) \
	{ \
		if (n > std::numeric_limits<FitSmallType>::max() || n < std::numeric_limits<FitSmallType>::min()) \
		{ \
			LIGHTS_BINARY_STORE_WRITER_APPEND_INTEGER_BODY(Type); \
		} \
		else \
		{ \
			return *this << static_cast<FitSmallType>(n); \
		} \
	}

	LIGHTS_BINARY_STORE_WRITER_APPEND_SIGNED_INTEGER(std::int16_t, std::int8_t);
	LIGHTS_BINARY_STORE_WRITER_APPEND_SIGNED_INTEGER(std::int32_t, std::int16_t);
	LIGHTS_BINARY_STORE_WRITER_APPEND_SIGNED_INTEGER(std::int64_t, std::int32_t);

#undef LIGHTS_BINARY_STORE_WRITER_APPEND_SIGNED_INTEGER

#define LIGHTS_BINARY_STORE_WRITER_APPEND_UNSIGNED_INTEGER(Type, FitSmallType) \
	BinaryStoreWriter& operator<< (Type n) \
	{ \
		if (n > std::numeric_limits<FitSmallType>::max()) \
		{ \
			LIGHTS_BINARY_STORE_WRITER_APPEND_INTEGER_BODY(Type); \
		} \
		else \
		{ \
			return *this << static_cast<FitSmallType>(n); \
		} \
	}
	LIGHTS_BINARY_STORE_WRITER_APPEND_UNSIGNED_INTEGER(std::uint16_t, std::uint8_t);
	LIGHTS_BINARY_STORE_WRITER_APPEND_UNSIGNED_INTEGER(std::uint32_t, std::uint16_t);
	LIGHTS_BINARY_STORE_WRITER_APPEND_UNSIGNED_INTEGER(std::uint64_t, std::uint32_t);

#undef LIGHTS_BINARY_STORE_WRITER_APPEND_UNSIGNED_INTEGER

#undef LIGHTS_BINARY_STORE_WRITER_APPEND_INTEGER_BODY

	/**
	 * It's only for write format to call and easy to restore.
	 * @note If the internal buffer is full will have no effect.
	 */
	template <typename T>
	void add_composed_type(const T& value)
	{
		if (m_state == FormatComposedTypeState::STARTED) // Reduce recursion.
		{
			make_format_sink_adapter(*this) << value;
		}
		else
		{
			m_composed_member_num = 0;
			std::uint8_t* type = m_buffer + m_length;
			auto composed_member_num = reinterpret_cast<std::uint16_t*>(m_buffer + m_length + sizeof(BinaryTypeCode));
			const auto composed_header_len = sizeof(BinaryTypeCode) + sizeof(std::uint16_t) / sizeof(std::uint8_t);
			m_length += composed_header_len;
			m_state = FormatComposedTypeState::STARTED;
			make_format_sink_adapter(*this) << value;
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

	/**
	 * Forward to @c lights::write() function.
	 * @note If the internal buffer is full will have no effect.
	 */
	template <typename Arg, typename ... Args>
	void write(StringView fmt, const Arg& value, const Args& ... args)
	{
		lights::write(make_format_sink_adapter(*this), fmt, value, args ...);
	}

	/**
	 * Forward to @c lights::write() function.
	 * @note If the internal buffer is full will have no effect.
	 */
	void write(StringView fmt)
	{
		lights::write(make_format_sink_adapter(*this), fmt);
	}

	/**
	 * Return the internal buffer.
	 */
	const std::uint8_t* data() const
	{
		return m_buffer;
	}

	/**
	 * Return the internal buffer in c string but not null-terminated.
	 */
	const char* c_str() const
	{
		return str_view().data();
	}

	/**
	 * Return a @c std::string that convert from internal buffer.
	 */
	std::string str() const
	{
		return str_view().to_string();
	}

	/**
	 * Return a @c StringView of internal buffer.
	 * @note The return value is only valid when this object have no change
	 *       the area of return @c StringView.
	 */
	StringView str_view() const
	{
		return { reinterpret_cast<const char*>(m_buffer), m_length };
	}

	/**
	 * Return the length of internal buffer.
	 */
	std::size_t length() const
	{
		return m_length;
	}

	/**
	 * Return the length of internal buffer.
	 * @details It's same as @c length() function.
	 */
	std::size_t size() const
	{
		return m_length;
	}

	/**
	 * Set the format result length to zero.
	 */
	void clear()
	{
		m_length = 0;
	}

	/**
	 * Return the max size that format result can be.
	 */
	constexpr std::size_t max_size() const
	{
		return buffer_size;
	}

	/**
	 * Return the internal buffer size.
	 */
	constexpr std::size_t capacity() const
	{
		return buffer_size;
	}

private:
	/**
	 * Check can append @c len data.
	 */
	bool can_append(std::size_t len)
	{
		return m_length + len <= max_size();
	}

	std::size_t m_length = 0;
	std::uint8_t m_buffer[buffer_size];
	FormatComposedTypeState m_state = FormatComposedTypeState::NO_INIT;
	std::uint16_t m_composed_member_num = 0;
	StringTablePtr m_str_table;
};


template <>
template <std::size_t buffer_size>
class FormatSinkAdapter<BinaryStoreWriter<buffer_size>>
{
public:
	explicit FormatSinkAdapter(BinaryStoreWriter<buffer_size>& sink) : m_sink(sink) {}

	void append(char ch)
	{
		m_sink.append(ch);
	}

	void append(std::size_t num, char ch)
	{
		for (std::size_t i = 0; i < num; ++i)
		{
			this->append(ch);
		}
	}

	void append(StringView str)
	{
		m_sink.append(str);
	}

	BinaryStoreWriter<buffer_size>& get_internal_sink()
	{
		return m_sink;
	}

private:
	BinaryStoreWriter<buffer_size>& m_sink;
};


/**
 * Use @c BinaryStoreWriter member function to format integer to speed up.
 */
#define LIGHTS_BINARY_STORE_WRITER_TO_STRING(Type) \
template <std::size_t buffer_size> \
inline void to_string(FormatSinkAdapter<BinaryStoreWriter<buffer_size>> out, Type n) \
{ \
	out.get_internal_sink() << n; \
}

LIGHTS_IMPLEMENT_ALL_INTEGER_FUNCTION(LIGHTS_BINARY_STORE_WRITER_TO_STRING)

#undef LIGHTS_BINARY_STORE_WRITER_TO_STRING


/**
 * BinaryRestoreWriter use to format text with arguments that store by @c BinaryStoreWriter.
 * @tparam buffer_size  Default size is @c WRITER_BUFFER_SIZE_DEFAULT
 * @note If the internal buffer is full will have no effect.
 */
template <std::size_t buffer_size = WRITER_BUFFER_SIZE_DEFAULT>
class BinaryRestoreWriter
{
public:
	BinaryRestoreWriter() = default;

	BinaryRestoreWriter(StringTablePtr str_table):
		m_str_table(str_table) {}

	/**
	 * @note If the internal buffer is full will have no effect.
	 */
	void append(char ch)
	{
		m_writer.append(ch);
	}

	/**
	 * @note If the internal buffer is full will have no effect.
	 */
	void append(StringView str)
	{
		m_writer.append(str);
	}

	/**
	 * Forward to lights::write() function to format text.
	 * @note If the internal buffer is full will have no effect.
	 */
	template <typename Arg, typename ... Args>
	void write_text(StringView fmt, const Arg& value, const Args& ... args)
	{
		// Must add namespace scope limit or cannot find suitable function.
		lights::write(make_format_sink_adapter(m_writer), fmt, value, args ...);
	}

	/**
	 * Forward to lights::write() function to format text.
	 * @note If the internal buffer is full will have no effect.
	 */
	void write_text(StringView fmt)
	{
		lights::write(make_format_sink_adapter(m_writer), fmt);
	}

	/**
	 * Forward to lights::write() function to format binary.
	 * @note If the internal buffer is full will have no effect.
	 */
	void write_binary(StringView fmt, const std::uint8_t* binary_store_args, std::size_t args_length);

	/**
	 * Forward to lights::operater<<() function.
	 * @return The reference of this object.
	 * @note If the internal buffer is full will have no effect.
	 */
	template <typename T>
	BinaryRestoreWriter& operator<< (const T& value)
	{
		make_format_sink_adapter(m_writer) << value;
		return *this;
	}

	/**
	 * Return a pointer to null-terminated character array that store in internal.
	 */
	const char* c_str() const
	{
		return m_writer.c_str();
	}

	/**
	 * Return a @c std::string that convert from internal buffer.
	 * @note This convertion will generate a data copy.
	 */
	std::string str() const
	{
		return m_writer.str();
	}

	/**
	 * Return a @c StringView of internal buffer.
	 * @note The return value is only valid when this object have no change
	 *       the area of return @c StringView.
	 */
	StringView str_view() const
	{
		return m_writer.str_view();
	}

	/**
	 * Return the length of internal buffer.
	 */
	std::size_t length() const
	{
		return m_writer.length();
	}

	/**
	 * Return the length of internal buffer.
	 * @details It's same as @c length() function.
	 */
	std::size_t size() const
	{
		return m_writer.size();
	}

	/**
	 * Set the format result length to zero.
	 */
	void clear()
	{
		m_writer.clear();
	}

	/**
	 * Return the max size that format result can be.
	 */
	constexpr std::size_t max_size() const
	{
		return m_writer.max_size();
	}

	/**
	 * Return the internal buffer size.
	 */
	constexpr std::size_t capacity() const
	{
		return m_writer.capacity();
	}

private:
	/**
	 * Write a argument and get the width of argument.
	 */
	std::uint8_t write_argument(const uint8_t* binary_store_args);

	TextWriter<buffer_size> m_writer;
	StringTablePtr m_str_table;
};


template <std::size_t buffer_size>
void BinaryRestoreWriter<buffer_size>::write_binary(StringView fmt, const std::uint8_t* binary_store_args, std::size_t args_length)
{
	if (args_length == 0)
	{
		append(fmt);
		return;
	}

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

	m_writer.append({fmt.data(), i});
	if (i < fmt.length())
	{
		auto width = write_argument(binary_store_args);
		fmt.move_forward(i + 2);
		write_binary(fmt, binary_store_args + width, args_length - width);
	}
}


template <std::size_t buffer_size>
std::uint8_t BinaryRestoreWriter<buffer_size>::write_argument(const std::uint8_t* binary_store_args)
{
	auto width = get_type_width(static_cast<BinaryTypeCode>(*binary_store_args));
	auto value_begin = binary_store_args + sizeof(BinaryTypeCode);
	switch (static_cast<BinaryTypeCode>(*binary_store_args))
	{
		case BinaryTypeCode::INVALID:
			break;
		case BinaryTypeCode::BOOL:
		{
			bool b = static_cast<bool>(*value_begin);
			m_writer << b;
			break;
		}
		case BinaryTypeCode::CHAR:
		{
			char ch = static_cast<char>(*value_begin);
			m_writer << ch;
			break;
		}
		case BinaryTypeCode::STRING:
		{
			width += binary_store_args[1];
			m_writer.append({reinterpret_cast<const char*>(&binary_store_args[2]), binary_store_args[1]});
			break;
		}
		case BinaryTypeCode::INT8_T:
		{
			auto p = reinterpret_cast<const int8_t*>(value_begin);
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::UINT8_T:
		{
			auto p = value_begin;
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::INT16_T:
		{
			auto p = reinterpret_cast<const int16_t*>(value_begin);
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::UINT16_T:
		{
			auto p = reinterpret_cast<const uint16_t*>(value_begin);
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::INT32_T:
		{
			auto p = reinterpret_cast<const int32_t*>(value_begin);
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::UINT32_T:
		{
			auto p = reinterpret_cast<const uint32_t*>(value_begin);
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::INT64_T:
		{
			auto p = reinterpret_cast<const int64_t*>(value_begin);
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::UINT64_T:
		{
			auto p = reinterpret_cast<const uint64_t*>(value_begin);
			m_writer << *p;
			break;
		}
		case BinaryTypeCode::COMPOSED_TYPE:
		{
			auto member_num = reinterpret_cast<const uint16_t*>(value_begin);
			for (std::size_t i = 0; i < *member_num; ++i)
			{
				width += write_argument(binary_store_args + sizeof(BinaryTypeCode) + width);
			}
			break;
		}
		case BinaryTypeCode::STRING_REF:
		{
			auto index = reinterpret_cast<const uint32_t*>(value_begin);
			if (m_str_table)
			{
				StringView str = m_str_table->get_str(*index);
				if (is_valid(str))
				{
					m_writer << str;
				}
				else
				{
					m_writer << "[[Invalid string index: " << *index << "]]";
				}
			}
			else
			{
				m_writer << "[[Use STRING_REF but string table is not set]]";
			}
			break;
		}
		case BinaryTypeCode::MAX:
			break;
	}
	return width + sizeof(BinaryTypeCode);
}

} // namespace lights
