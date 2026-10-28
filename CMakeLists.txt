cmake_minimum_required(VERSION 3.16)
project(perovsat_protocol LANGUAGES C)

add_library(perovsat_protocol_core
    core/reader.c
    core/writer.c
)
target_include_directories(perovsat_protocol_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(perovsat_protocol_core PUBLIC c_std_11)

add_library(perovsat_protocol_flight
    flight/imu/imu.c
    flight/sweep/sweep.c
    flight/payload_reading/payload_reading.c
)
target_include_directories(perovsat_protocol_flight PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(perovsat_protocol_flight PUBLIC c_std_11)
target_link_libraries(perovsat_protocol_flight PUBLIC perovsat_protocol_core)

# Convenient aggregate target for consumers that use all flight value types.
add_library(perovsat_protocol INTERFACE)
target_link_libraries(perovsat_protocol INTERFACE perovsat_protocol_flight)

option(PEROVSAT_PROTOCOL_BUILD_TESTS "Build host tests and examples" OFF)
if(PEROVSAT_PROTOCOL_BUILD_TESTS)
    enable_language(CXX)
    enable_testing()

    add_executable(reader_test core/reader_test.cpp)
    target_compile_features(reader_test PRIVATE cxx_std_17)
    target_link_libraries(reader_test PRIVATE perovsat_protocol_core)
    add_test(NAME reader_test COMMAND reader_test)

    add_executable(writer_test core/writer_test.cpp)
    target_compile_features(writer_test PRIVATE cxx_std_17)
    target_link_libraries(writer_test PRIVATE perovsat_protocol_core)
    add_test(NAME writer_test COMMAND writer_test)

    add_executable(imu_test flight/imu/imu_test.cpp)
    target_compile_features(imu_test PRIVATE cxx_std_17)
    target_link_libraries(imu_test PRIVATE perovsat_protocol_flight)
    add_test(NAME imu_test COMMAND imu_test)

    add_executable(sweep_test flight/sweep/sweep_test.cpp)
    target_compile_features(sweep_test PRIVATE cxx_std_17)
    target_link_libraries(sweep_test PRIVATE perovsat_protocol_flight)
    add_test(NAME sweep_test COMMAND sweep_test)

    add_executable(payload_reading_test flight/payload_reading/payload_reading_test.cpp)
    target_compile_features(payload_reading_test PRIVATE cxx_std_17)
    target_link_libraries(payload_reading_test PRIVATE perovsat_protocol_flight)
    add_test(NAME payload_reading_test COMMAND payload_reading_test)

    add_executable(payload_reading_example examples/payload_reading.c)
    target_link_libraries(payload_reading_example PRIVATE perovsat_protocol)
    add_test(NAME payload_reading_example COMMAND payload_reading_example)

    add_executable(cpp_consumer_example examples/cpp_consumer.cpp)
    target_compile_features(cpp_consumer_example PRIVATE cxx_std_17)
    target_link_libraries(cpp_consumer_example PRIVATE perovsat_protocol)
    add_test(NAME cpp_consumer_example COMMAND cpp_consumer_example)
endif()
