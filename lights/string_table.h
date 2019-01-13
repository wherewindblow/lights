/**
 * string_table.h
 * @author wherewindblow
 * @date   Aug 30, 2017
 */

#pragma once

#include <memory>

#include "sequence.h"


namespace lights {

namespace details {

struct StringTableImpl;

} // namespace details

class StringTable;
using StringTablePtr = std::shared_ptr<StringTable>;


/**
 * StringTable record string with index.
 */
class StringTable
{
public:
	/**
	 * Creates string table.
	 */
	static StringTablePtr create(StringView filename)
	{
		return std::make_shared<StringTable>(filename);
	}

	/**
	 * Creates string table.
	 */
	StringTable(StringView filename);

	/**
	 * Destroys string table.
	 */
	~StringTable();

	/**
	 * Gets index of string. If cannot find string will add it and get new index.
	 */
	std::size_t get_index(StringView str);

	/**
	 * Add new string.
	 * @return Returns index of string.
	 */
	std::size_t add_str(StringView str);

	/**
	 * Get string by index.
	 * @note Return nullptr if index is invalid.
	 */
	StringView get_str(std::size_t index) const;

	/**
	 * Get string by index.
	 * @note Return nullptr if index is invalid.
	 */
	StringView operator[] (std::size_t index) const
	{
		return get_str(index);
	}

private:
	using ImplementType = details::StringTableImpl;
	/**
	 * It's same as use @c p_impl directly. It's the workaround way with Clion and give
	 * type suggestion to code completion.
	 */
	ImplementType* impl()
	{
		return p_impl.get();
	}

	const ImplementType* impl() const
	{
		return p_impl.get();
	}

	std::unique_ptr<ImplementType> p_impl;
};

} // namespace lights
