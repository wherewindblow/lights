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

#define LIGHTS_LINE_ENDER "\n"

#define LIGHTS_INTERNAL_DEBUG(format, ...) \
	std::printf("file: %s, line: %d, func: %s > " format LIGHTS_LINE_ENDER, __FILE__, __LINE__, __func__, __VA_ARGS__);

#define LIGHTS_OPEN_LOG
