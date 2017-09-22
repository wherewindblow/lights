/**
 * config.h
 * @author wherewindblow
 * @date   Dec 24, 2016
 */

#pragma once

/**
 * Optimize integer format.
 * @note Only have effect on gcc -O2 option, on -O0 option will have bad effect.
 */
#define LIGHTS_DETAILS_INTEGER_FORMATER_OPTIMIZE

#define LIGHTS_INTERNAL_DEBUG(format, ...) \
	std::printf("file: %s, line: %d, func: %s > " format "\n", __FILE__, __LINE__, __func__, __VA_ARGS__);

/**
 * Define this macro to open LIGHTS_LOG to expand in compile time
 * or disable all log that use with LIGHTS_LOG macro.
 */
#define LIGHTS_OPEN_LOG

/**
 * Define accsertion failure behavior by level.
 * 1, failure will lead to core dump.
 * 2, failure will lead to throw a assertion error exception.
 * others value, assertion have no effect.
 */
#define LIGHTS_OPEN_ASSERTION 1
