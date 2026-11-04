#include "sample.h"

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
    uint8_t bytes[PSP_SAMPLE_MAX_SERIALIZED_SIZE];
    psp_writer writer;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_sample_serialize(&writer, nullptr) == PSP_INVALID_ARGUMENT);

    psp_sample input{};
    input.kind = PSP_SAMPLE_IMU_STIMULUS;
    input.body.imu_stimulus.sequence = 7;
    input.body.imu_stimulus.accel_m_s2[2] = 9.5f;
    input.body.imu_stimulus.temperature_present = 1;
    input.body.imu_stimulus.temperature_c = 25.f;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_sample_serialize(&writer, &input) == PSP_OK);
    CHECK(psp_writer_position(&writer) == PSP_SAMPLE_MAX_SERIALIZED_SIZE);

    psp_sample output{};
    psp_reader reader;
    psp_reader_init(&reader, bytes, psp_writer_position(&writer));
    CHECK(psp_sample_deserialize(&reader, &output) == PSP_OK);
    CHECK(psp_reader_finish(&reader) == PSP_OK);
    CHECK(output.kind == PSP_SAMPLE_IMU_STIMULUS);
    CHECK(output.body.imu_stimulus.sequence == 7);
    CHECK(output.body.imu_stimulus.accel_m_s2[2] == 9.5f);
    CHECK(output.body.imu_stimulus.temperature_c == 25.f);

    input.kind = (psp_sample_kind)9;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_sample_serialize(&writer, &input) == PSP_INVALID_VALUE);
    uint8_t unknown[] = {9};
    psp_reader_init(&reader, unknown, sizeof(unknown));
    CHECK(psp_sample_deserialize(&reader, &output) == PSP_INVALID_VALUE);
    return 0;
}
