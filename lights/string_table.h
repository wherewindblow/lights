/**
 * string_table.h
 * @author wherewindblow
 * @date   Aug 30, 2017
 */

#pragma once

#include <memory>

#include "env.h"
#include "sequence.h"


namespace lights {

namespace details {

struct StringTableImpl;

} // namespace details


class StringTable
{
public:
	using StringViewPtr = std::shared_ptr<const StringView>;
	using StringTablePtr = std::shared_ptr<StringTable>;

	static StringTablePtr create(StringView filename)
	{
		return std::make_shared<StringTable>(filename);
	}

	StringTable(StringView filename);

	~StringTable();

	std::size_t get_index(StringView str);

	std::size_t add_str(StringView str);

	StringView get_str(std::size_t index) const;

	StringView operator[] (std::size_t index) const
	{
		return get_str(index);
	}

private:
	using ImplementType = details::StringTableImpl;
	/**
	 * It's same as use @c p_impl directly. It's the workaround way with Clion and give
	 * type sugguestion to code completion.
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

using StringTablePtr = StringTable::StringTablePtr;

} // namespace lights
