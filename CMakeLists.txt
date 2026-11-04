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

add_library(perovsat_protocol_simulation
    simulation/frame/frame.c
    simulation/frame/command/command.c
    simulation/frame/command/utc_sync/utc_sync.c
    simulation/frame/sample/sample.c
    simulation/frame/sample/imu_stimulus/imu_stimulus.c
    simulation/frame/log/log.c
    simulation/frame/log/utc_report/utc_report.c
    simulation/frame/log/i2c_transaction/i2c_transaction.c
    simulation/frame/log/result/result.c
    simulation/frame/log/status/status.c
)
target_include_directories(perovsat_protocol_simulation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(perovsat_protocol_simulation PUBLIC c_std_11)
target_link_libraries(perovsat_protocol_simulation PUBLIC perovsat_protocol_core)

# Convenient aggregate target for consumers that use flight and simulation types.
add_library(perovsat_protocol INTERFACE)
target_link_libraries(perovsat_protocol INTERFACE
    perovsat_protocol_flight
    perovsat_protocol_simulation
)

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

    add_executable(utc_sync_test simulation/frame/command/utc_sync/utc_sync_test.cpp)
    target_compile_features(utc_sync_test PRIVATE cxx_std_17)
    target_link_libraries(utc_sync_test PRIVATE perovsat_protocol_simulation)
    add_test(NAME utc_sync_test COMMAND utc_sync_test)

    add_executable(command_test simulation/frame/command/command_test.cpp)
    target_compile_features(command_test PRIVATE cxx_std_17)
    target_link_libraries(command_test PRIVATE perovsat_protocol_simulation)
    add_test(NAME command_test COMMAND command_test)

    add_executable(imu_stimulus_test
        simulation/frame/sample/imu_stimulus/imu_stimulus_test.cpp)
    target_compile_features(imu_stimulus_test PRIVATE cxx_std_17)
    target_link_libraries(imu_stimulus_test PRIVATE perovsat_protocol_simulation)
    add_test(NAME imu_stimulus_test COMMAND imu_stimulus_test)

    add_executable(sample_test simulation/frame/sample/sample_test.cpp)
    target_compile_features(sample_test PRIVATE cxx_std_17)
    target_link_libraries(sample_test PRIVATE perovsat_protocol_simulation)
    add_test(NAME sample_test COMMAND sample_test)

    add_executable(utc_report_test simulation/frame/log/utc_report/utc_report_test.cpp)
    target_compile_features(utc_report_test PRIVATE cxx_std_17)
    target_link_libraries(utc_report_test PRIVATE perovsat_protocol_simulation)
    add_test(NAME utc_report_test COMMAND utc_report_test)

    add_executable(i2c_transaction_test
        simulation/frame/log/i2c_transaction/i2c_transaction_test.cpp)
    target_compile_features(i2c_transaction_test PRIVATE cxx_std_17)
    target_link_libraries(i2c_transaction_test PRIVATE perovsat_protocol_simulation)
    add_test(NAME i2c_transaction_test COMMAND i2c_transaction_test)

    add_executable(log_test simulation/frame/log/log_test.cpp)
    target_compile_features(log_test PRIVATE cxx_std_17)
    target_link_libraries(log_test PRIVATE perovsat_protocol_simulation)
    add_test(NAME log_test COMMAND log_test)

    add_executable(frame_test simulation/frame/frame_test.cpp)
    target_compile_features(frame_test PRIVATE cxx_std_17)
    target_link_libraries(frame_test PRIVATE perovsat_protocol_simulation)
    add_test(NAME frame_test COMMAND frame_test)

    add_executable(golden_vectors_test simulation/golden_vectors_test.cpp)
    target_compile_features(golden_vectors_test PRIVATE cxx_std_17)
    target_compile_definitions(golden_vectors_test PRIVATE
        PSP_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(golden_vectors_test PRIVATE perovsat_protocol_simulation)
    add_test(NAME golden_vectors_test COMMAND golden_vectors_test)

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
