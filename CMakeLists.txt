cmake_minimum_required(VERSION 3.16)
project(perovsat_protocol LANGUAGES CXX)

add_library(perovsat_protocol INTERFACE)
target_include_directories(perovsat_protocol INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(perovsat_protocol INTERFACE cxx_std_17)

option(PEROVSAT_PROTOCOL_BUILD_TESTS "Build standalone protocol tests and example" OFF)
if(PEROVSAT_PROTOCOL_BUILD_TESTS)
    enable_testing()
    add_executable(payload_reading_test tests/payload_reading_test.cpp)
    target_link_libraries(payload_reading_test PRIVATE perovsat_protocol)
    add_test(NAME payload_reading COMMAND payload_reading_test)

    add_executable(payload_reading_example examples/payload_reading.cpp)
    target_link_libraries(payload_reading_example PRIVATE perovsat_protocol)
    add_test(NAME payload_reading_example COMMAND payload_reading_example)
endif()
