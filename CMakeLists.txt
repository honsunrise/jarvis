cmake_minimum_required(VERSION 3.0)
project(jarvis)

set(CMAKE_CXX_STANDARD 11)

set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     ON)

find_package(Boost 1.58.0 COMPONENTS iostreams date_time filesystem system program_options thread log)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

file(GLOB_RECURSE SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/* ${PROJECT_SOURCE_DIR}/include/*)
include_directories(include)
include_directories(json/src)
include_directories(filter_audio)
link_directories(filter_audio)
link_directories(/home/zhsyourai/sdks/xf_sdk/libs/x64)

include_directories(src)

add_executable(jarvis ${SOURCE_FILES})
target_link_libraries(jarvis pthread dl rt asound msc filteraudio)

if(Boost_FOUND)
    target_link_libraries(jarvis ${Boost_LIBRARIES})
endif()

# Test
enable_testing()
add_subdirectory(test)
