#include "imu.h"
#include <cstdio>
#include <cstdlib>

#define CHECK(condition) do { if (!(condition)) { \
    std::fprintf(stderr, "failed line %d: %s\n", __LINE__, #condition); std::exit(1); \
} } while (false)

int main() {
    uint8_t bytes[PSP_IMU_READING_MAX_SERIALIZED_SIZE]{};
    psp_writer writer;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_sensor_value_serialize(&writer, nullptr) == PSP_INVALID_ARGUMENT);
    CHECK(psp_writer_status(&writer) == PSP_INVALID_ARGUMENT);

    psp_imu_reading input{};
    input.gyro[0] = {-2, -500000};
    input.gyro[1] = {INT32_MIN, INT32_MAX};
    input.accel[2] = {9, 500000};
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_imu_reading_serialize(&writer, &input) == PSP_OK);
    CHECK(bytes[0] == 0xfe && bytes[4] == 0xe0);

    psp_reader reader;
    psp_reader_init(&reader, bytes, sizeof(bytes));
    psp_imu_reading output{};
    CHECK(psp_imu_reading_deserialize(&reader, &output) == PSP_OK);
    CHECK(output.gyro[0].whole == -2 && output.gyro[0].micro == -500000);
    CHECK(output.gyro[1].whole == INT32_MIN && output.gyro[1].micro == INT32_MAX);
    CHECK(output.accel[2].micro == 500000);
}
