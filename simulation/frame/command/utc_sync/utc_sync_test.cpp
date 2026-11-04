#include "utc_sync.h"

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
    uint8_t bytes[PSP_COMMAND_UTC_SYNC_MAX_SERIALIZED_SIZE];
    psp_writer writer;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_command_utc_sync_serialize(&writer, nullptr) ==
          PSP_INVALID_ARGUMENT);
    CHECK(psp_command_utc_sync_serialize(nullptr, nullptr) ==
          PSP_INVALID_ARGUMENT);

    psp_command_utc_sync input{};
    input.offset_correction_ns = -1500;
    input.rate_ppb = -100000;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_command_utc_sync_serialize(&writer, &input) == PSP_OK);
    CHECK(psp_writer_position(&writer) == PSP_COMMAND_UTC_SYNC_MAX_SERIALIZED_SIZE);

    psp_command_utc_sync output{};
    psp_reader reader;
    psp_reader_init(&reader, bytes, psp_writer_position(&writer));
    CHECK(psp_command_utc_sync_deserialize(&reader, &output) == PSP_OK);
    CHECK(psp_reader_finish(&reader) == PSP_OK);
    CHECK(output.offset_correction_ns == -1500);
    CHECK(output.rate_ppb == -100000);

    psp_reader_init(&reader, bytes, 0);
    CHECK(psp_command_utc_sync_deserialize(&reader, nullptr) ==
          PSP_INVALID_ARGUMENT);
    CHECK(psp_command_utc_sync_deserialize(nullptr, nullptr) ==
          PSP_INVALID_ARGUMENT);
    return 0;
}
