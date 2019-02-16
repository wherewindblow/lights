set(LIGHTS_SOURCE_FILES
        config.h
        env.h
        sequence.h
        sink.h
        non_copyable.h
        format.h format.cpp
        ostream.h
        file.h
        string_table.h string_table.cpp
        logger.h logger.cpp
        common.h
        current_function.hpp
        exception.h exception.cpp
        precise_time.h precise_time.cpp

        format/binary_format.h format/binary_format.cpp
        sinks/stdout_sink.h
        sinks/cout_sink.h
        sinks/null_sink.h
        sinks/file_sink.h sinks/file_sink.cpp)

add_library(lights_shared SHARED ${LIGHTS_SOURCE_FILES})
add_library(lights_static STATIC ${LIGHTS_SOURCE_FILES})

set_target_properties(lights_shared PROPERTIES OUTPUT_NAME "lights")
set_target_properties(lights_static PROPERTIES OUTPUT_NAME "lights")
