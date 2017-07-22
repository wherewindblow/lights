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
	printf("file: %s, line: %d, func: %s > " format "\n", __FILE__, __LINE__, __func__, __VA_ARGS__);

#define LIGHTS_LOG(logger, level, module, ...) \
	logger.log(level, module, __FILE__, __func__, __LINE__, __VA_ARGS__);
#define LIGHTS_DEBUG(logger, module, ...) \
	LIGHTS_LOG(logger, lights::LogLevel::DEBUG, module, __VA_ARGS__);
#define LIGHTS_INFO(logger, module, ...) \
	LIGHTS_LOG(logger, lights::LogLevel::INFO, module, __VA_ARGS__);
#define LIGHTS_WARN(logger, module, ...) \
	LIGHTS_LOG(logger, lights::LogLevel::WARN, module, __VA_ARGS__);
#define LIGHTS_ERROR(logger, module, ...) \
	LIGHTS_LOG(logger, lights::LogLevel::ERROR, module, __VA_ARGS__);
