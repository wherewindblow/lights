set(LIGHTS_SOURCE_FILES
        config.h
        env.h
        sequence.h
        sink.h
        format.h format.cpp
        ostream.h
        file.h
        string_table.h string_table.cpp
        logger.h logger.cpp
        common.h
        current_function.hpp
        exception.h exception.cpp

        format/binary_format.h format/binary_format.cpp
        log_sinks/stdout_sink.h
        log_sinks/cout_sink.h
        log_sinks/null_sink.h
        log_sinks/file_sink.h log_sinks/file_sink.cpp)

add_library(lights_shared SHARED ${LIGHTS_SOURCE_FILES})
add_library(lights_static STATIC ${LIGHTS_SOURCE_FILES})

set_target_properties(lights_shared PROPERTIES OUTPUT_NAME "lights")
set_target_properties(lights_static PROPERTIES OUTPUT_NAME "lights")
