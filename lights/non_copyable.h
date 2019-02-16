/**
 * non_copyable.h
 * @author wherewindblow
 * @date   Feb 15, 2019
 */

#pragma once


namespace lights {

/**
 * Disable copy operation with object.
 */
class NonCopyable
{
public:
	/**
	 * Default constructor.
	 */
	NonCopyable() = default;

	/**
	 * Disable copy constructor.
	 */
	NonCopyable(const NonCopyable&) = delete;

	/**
	 * Disable assignment operator.
	 */
	NonCopyable& operator=(const NonCopyable&) = delete;
};

} // namespace lights
