set(LIGHTS_SOURCE_FILES
        config.h
        block_description.h
        sink_adapter.h
        format.h
        file.h
        logger.h logger.cpp
        common.h
        current_function.hpp
        exception.h exception.cpp

        log_sinks/stdout_sink.h
        log_sinks/cout_sink.h
        log_sinks/null_sink.h
        log_sinks/file_sink.h log_sinks/file_sink.cpp)

add_library(lights_shared SHARED ${LIGHTS_SOURCE_FILES})
add_library(lights_static STATIC ${LIGHTS_SOURCE_FILES})

set_target_properties(lights_shared PROPERTIES OUTPUT_NAME "lights")
set_target_properties(lights_static PROPERTIES OUTPUT_NAME "lights")
