# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in ${CMAKE_BINARY_DIR}/googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
        ${CMAKE_BINARY_DIR}/googletest-build)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
endif()

set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

find_package(Boost 1.58.0 COMPONENTS iostreams date_time filesystem system program_options thread log)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

include_directories(../src)

set(SOURCE_HTTP_FILES http/http-tests.cpp)
add_executable(http-tests ${SOURCE_HTTP_FILES})
target_link_libraries(http-tests gtest_main)
add_test(NAME http-tests COMMAND http-tests)

set(SOURCE_HTTP_FILES tts-tests.cpp ../src/tts/engine/IflytekTTS.cpp ../src/tts/TTS.cpp ../src/Voice.cpp ../src/VoicePlayer.cpp)
add_executable(tts-tests ${SOURCE_HTTP_FILES})
target_link_libraries(tts-tests gtest_main msc asound)
add_test(NAME tts-tests COMMAND tts-tests)

set(SOURCE_HTTP_FILES action-gen-tests.cpp ../src/nlp/NLP.cpp ../src/nlp/engine/LPTProcessor.cpp ../src/nlp/ActionAnalytics.cpp)
add_executable(action-gen-tests ${SOURCE_HTTP_FILES})
target_link_libraries(action-gen-tests gtest_main)
add_test(NAME action-gen-tests COMMAND action-gen-tests)


if(Boost_FOUND)
    target_link_libraries(http-tests ${Boost_LIBRARIES})
    target_link_libraries(tts-tests ${Boost_LIBRARIES})
    target_link_libraries(action-gen-tests ${Boost_LIBRARIES})
endif()