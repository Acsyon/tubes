# cmake version should not matter (?)
cmake_minimum_required(VERSION 3.0)
project(tubes LANGUAGES C)

# Binaries go to "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Link math library for GCC
if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    link_libraries(m)
endif ()

# Target architecture
if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
endif ()

# Adjust __FILE__ prefix
if (
        CMAKE_C_COMPILER_ID STREQUAL "GNU" AND 
        CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 8
    )
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
        -fmacro-prefix-map=${CMAKE_SOURCE_DIR}/code/src/= \
    ")
endif ()

# Set C standard and compiler flags
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED YES)
set(CMAKE_C_EXTENSIONS OFF)
if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG -O2 -flto -fwhole-program")
    set(CMAKE_C_FLAGS_DEBUG "-g -Wformat=1 -Werror")
elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Wall /Za")
    set(CMAKE_C_FLAGS_RELEASE "/DNDEBUG /Ox /LTCG")
    set(CMAKE_C_FLAGS_DEBUG "/Z7 /WX")
endif ()

# Set include directory and source files
set(SOURCE_FILES
    src/main.c
    src/gameinfo.c
    src/input.c
    src/log.c
    src/options.c
    src/seed.c
    src/tube.c
)

# Compile
add_executable("${PROJECT_NAME}" ${SOURCE_FILES})

