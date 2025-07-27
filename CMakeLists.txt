cmake_minimum_required(VERSION 3.31)
project(Synthesizer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directories
include_directories(
        include
        include/Audio
        include/Audio/Midi
        include/Utils
        include/rapidjson
)

# Collect source files
file(GLOB_RECURSE SOURCES
        src/*.cpp
)

# Create the executable
add_executable(Synthesizer ${SOURCES})
target_link_options(Synthesizer PRIVATE "-Wl,--subsystem,console")