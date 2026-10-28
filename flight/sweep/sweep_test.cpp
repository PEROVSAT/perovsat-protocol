#include "sweep.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>

#define CHECK(condition) do { if (!(condition)) { \
    std::fprintf(stderr, "failed line %d: %s\n", __LINE__, #condition); std::exit(1); \
} } while (false)

int main() {
    uint8_t bytes[PSP_SWEEP_MAX_SERIALIZED_SIZE]{};
    psp_sweep input{};
    input.meta.timestamp = 0x12345678;
    input.iv.voltage[39] = -0.0f;
    psp_writer writer;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_sweep_serialize(&writer, &input) == PSP_OK);
    CHECK(psp_writer_position(&writer) == sizeof(bytes));
    CHECK(bytes[40] == 0x78 && bytes[207] == 0x80);

    psp_reader reader;
    psp_reader_init(&reader, bytes, sizeof(bytes));
    psp_sweep output{};
    CHECK(psp_sweep_deserialize(&reader, &output) == PSP_OK);
    CHECK(output.meta.timestamp == input.meta.timestamp);
    CHECK(std::signbit(output.iv.voltage[39]));
}
