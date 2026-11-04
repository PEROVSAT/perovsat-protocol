#include "command.h"

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
    uint8_t bytes[PSP_COMMAND_MAX_SERIALIZED_SIZE];
    psp_writer writer;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_command_serialize(&writer, nullptr) == PSP_INVALID_ARGUMENT);

    psp_command input{};
    input.kind = PSP_COMMAND_UTC_SYNC;
    input.request_id = 9;
    input.body.utc_sync.offset_correction_ns = -1500;
    input.body.utc_sync.rate_ppb = -100000;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_command_serialize(&writer, &input) == PSP_OK);
    CHECK(psp_writer_position(&writer) == PSP_COMMAND_MAX_SERIALIZED_SIZE);

    psp_command output{};
    psp_reader reader;
    psp_reader_init(&reader, bytes, psp_writer_position(&writer));
    CHECK(psp_command_deserialize(&reader, &output) == PSP_OK);
    CHECK(psp_reader_finish(&reader) == PSP_OK);
    CHECK(output.kind == PSP_COMMAND_UTC_SYNC);
    CHECK(output.request_id == 9);
    CHECK(output.body.utc_sync.offset_correction_ns == -1500);
    CHECK(output.body.utc_sync.rate_ppb == -100000);

    input.kind = PSP_COMMAND_GET_STATUS;
    input.request_id = 0x11223344;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_command_serialize(&writer, &input) == PSP_OK);
    CHECK(psp_writer_position(&writer) == 5);
    psp_reader_init(&reader, bytes, psp_writer_position(&writer));
    CHECK(psp_command_deserialize(&reader, &output) == PSP_OK);
    CHECK(psp_reader_finish(&reader) == PSP_OK);
    CHECK(output.kind == PSP_COMMAND_GET_STATUS);
    CHECK(output.request_id == 0x11223344);

    input.kind = (psp_command_kind)9;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_command_serialize(&writer, &input) == PSP_INVALID_VALUE);
    uint8_t unknown[] = {9};
    psp_reader_init(&reader, unknown, sizeof(unknown));
    CHECK(psp_command_deserialize(&reader, &output) == PSP_INVALID_VALUE);
    return 0;
}
