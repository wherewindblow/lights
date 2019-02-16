/**
 * string_table.h
 * @author wherewindblow
 * @date   Aug 30, 2017
 */

#pragma once

#include "sequence.h"
#include "non_copyable.h"


namespace lights {

namespace details {

struct StringTableImpl;

} // namespace details


/**
 * StringTable record string with index.
 */
class StringTable : public NonCopyable
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
	 * Adds new string.
	 * @return Returns index of string.
	 */
	std::size_t add_str(StringView str);

	/**
	 * Gets string by index.
	 * @note Return nullptr if index is invalid.
	 */
	StringView get_str(std::size_t index) const;

	/**
	 * Gets string by index.
	 * @note Return nullptr if index is invalid.
	 */
	StringView operator[] (std::size_t index) const;

private:
	using ImplementType = details::StringTableImpl;
	ImplementType* p_impl;
};


// ========================== Inline implement. ==============================

inline StringView StringTable::operator[](std::size_t index) const
{
	return get_str(index);
}

} // namespace lights
