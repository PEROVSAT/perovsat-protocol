#include "imu_stimulus.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#define CHECK(condition)                                                       \
    do {                                                                       \
        if (!(condition)) {                                                    \
            std::fprintf(stderr, "failed line %d: %s\n", __LINE__,             \
                         #condition);                                          \
            std::exit(1);                                                      \
        }                                                                      \
    } while (false)

int main() {
    uint8_t bytes[PSP_SAMPLE_IMU_STIMULUS_MAX_SERIALIZED_SIZE];
    psp_writer writer;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_sample_imu_stimulus_serialize(&writer, nullptr) ==
          PSP_INVALID_ARGUMENT);

    psp_sample_imu_stimulus input{};
    input.sequence = 7;
    input.accel_m_s2[2] = 9.5f;
    input.gyro_rad_s[0] = -1.f;
    input.gyro_rad_s[1] = 0.5f;
    input.temperature_present = 0;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_sample_imu_stimulus_serialize(&writer, &input) == PSP_OK);
    const size_t without_temperature = psp_writer_position(&writer);
    CHECK(without_temperature + 4u == PSP_SAMPLE_IMU_STIMULUS_MAX_SERIALIZED_SIZE);

    psp_sample_imu_stimulus output{};
    psp_reader reader;
    psp_reader_init(&reader, bytes, without_temperature);
    CHECK(psp_sample_imu_stimulus_deserialize(&reader, &output) == PSP_OK);
    CHECK(psp_reader_finish(&reader) == PSP_OK);
    CHECK(output.sequence == 7);
    CHECK(output.accel_m_s2[2] == 9.5f);
    CHECK(output.gyro_rad_s[0] == -1.f);
    CHECK(output.gyro_rad_s[1] == 0.5f);
    CHECK(output.temperature_present == 0);
    CHECK(output.temperature_c == 0.f);

    input.temperature_present = 1;
    input.temperature_c = 25.f;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_sample_imu_stimulus_serialize(&writer, &input) == PSP_OK);
    CHECK(psp_writer_position(&writer) == PSP_SAMPLE_IMU_STIMULUS_MAX_SERIALIZED_SIZE);
    psp_reader_init(&reader, bytes, psp_writer_position(&writer));
    CHECK(psp_sample_imu_stimulus_deserialize(&reader, &output) == PSP_OK);
    CHECK(psp_reader_finish(&reader) == PSP_OK);
    CHECK(output.temperature_present == 1);
    CHECK(output.temperature_c == 25.f);

    input.temperature_present = 2;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_sample_imu_stimulus_serialize(&writer, &input) == PSP_INVALID_VALUE);
    return 0;
}
