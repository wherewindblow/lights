/**
 * string_table.h
 * @author wherewindblow
 * @date   Aug 30, 2017
 */

#pragma once

#include "sequence.h"


namespace lights {

namespace details {

struct StringTableImpl;

} // namespace details


/**
 * StringTable record string with index.
 */
class StringTable
{
public:
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
	ImplementType* p_impl;
};

} // namespace lights
