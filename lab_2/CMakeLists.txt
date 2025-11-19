cmake_minimum_required(VERSION 3.10)
project(lab_2 C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pthread -lm")

# Add compiler flags to a variable that can be exported
set(COMPILE_FLAGS "-Wall -Wextra -pthread -lm")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILE_FLAGS}")

# Export compile flags
add_custom_target(export_compile_flags
    COMMAND echo "${COMPILE_FLAGS}" > compile_flags.txt
    COMMENT "Exporting compile flags to compile_flags.txt"
)

# Source files
set(SOURCES
    src/main.c
    src/filters.c
    src/matrix_utils.c
    src/thread_utils.c
)

# Include directories
include_directories(include)

# Create executable - ИМЯ ДОЛЖНО СОВПАДАТЬ С PROJECT NAME ИЛИ БЫТь matrix_processor
add_executable(lab_2 ${SOURCES})

# Link libraries
target_link_libraries(lab_2 m pthread)

# Add dependency to export compile flags
add_dependencies(lab_2 export_compile_flags)

# Installation (optional)
install(TARGETS lab_2 DESTINATION bin)
