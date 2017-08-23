# Is's only use to check can pass compile.

include_directories(..)

set(LIGHTS_EXAMPLE_SRC
        example_main.cpp
        example_format.h example_format.cpp
        example_log.h example_log.cpp)

add_executable(lights_example ${LIGHTS_EXAMPLE_SRC})
target_link_libraries(lights_example lights_static)
