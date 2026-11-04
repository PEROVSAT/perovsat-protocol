#include "frame.h"

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
    CHECK(psp_address_accepted(PSP_ADDRESS_ALL, 1) == 1);
    CHECK(psp_address_accepted(PSP_ADDRESS_ALL, 2) == 1);
    CHECK(psp_address_accepted(1, 1) == 1);
    CHECK(psp_address_accepted(1, 2) == 0);

    uint8_t bytes[PSP_FRAME_MAX_SERIALIZED_SIZE];
    psp_writer writer;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_frame_serialize(&writer, nullptr) == PSP_INVALID_ARGUMENT);
    CHECK(psp_writer_status(&writer) == PSP_INVALID_ARGUMENT);

    psp_frame input{};
    input.version = 0;
    input.kind = PSP_FRAME_COMMAND;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_frame_serialize(&writer, &input) == PSP_INVALID_VALUE);
    CHECK(psp_writer_position(&writer) == 0);

    input.version = (uint8_t)(PSP_SIMULATION_VERSION + 1u);
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_frame_serialize(&writer, &input) == PSP_INVALID_VALUE);
    CHECK(psp_writer_position(&writer) == 0);

    input.version = PSP_SIMULATION_VERSION;
    input.destination = PSP_ADDRESS_ALL;
    input.source = 1;
    input.time_sent = 0x0807060504030201ull;
    input.time_received = 0;
    input.kind = PSP_FRAME_COMMAND;
    input.body.command.kind = PSP_COMMAND_UTC_SYNC;
    input.body.command.request_id = 9;
    input.body.command.body.utc_sync.offset_correction_ns = -1500;
    input.body.command.body.utc_sync.rate_ppb = -100000;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_frame_serialize(&writer, &input) == PSP_OK);
    CHECK(bytes[0] == PSP_SIMULATION_VERSION);
    CHECK(bytes[1] == PSP_ADDRESS_ALL);
    CHECK(bytes[3] == 0x01 && bytes[10] == 0x08);
    CHECK(bytes[PSP_FRAME_PREFIX_MAX_SERIALIZED_SIZE - 1] == PSP_FRAME_COMMAND);

    psp_frame output{};
    psp_reader reader;
    psp_reader_init(&reader, bytes, psp_writer_position(&writer));
    CHECK(psp_frame_deserialize(&reader, &output) == PSP_OK);
    CHECK(psp_reader_finish(&reader) == PSP_OK);
    CHECK(output.version == PSP_SIMULATION_VERSION);
    CHECK(output.destination == PSP_ADDRESS_ALL);
    CHECK(output.source == 1);
    CHECK(psp_address_accepted(output.destination, 1) == 1);
    CHECK(output.time_sent == input.time_sent);
    CHECK(output.time_received == 0);
    CHECK(output.kind == PSP_FRAME_COMMAND);
    CHECK(output.body.command.request_id == 9);

    output.time_received = 42;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_frame_serialize(&writer, &output) == PSP_OK);
    psp_reader_init(&reader, bytes, psp_writer_position(&writer));
    CHECK(psp_frame_deserialize(&reader, &output) == PSP_OK);
    CHECK(output.time_received == 42);

    bytes[0] = (uint8_t)(PSP_SIMULATION_VERSION + 1u);
    psp_reader_init(&reader, bytes, psp_writer_position(&writer));
    CHECK(psp_frame_deserialize(&reader, &output) == PSP_INVALID_VALUE);

    input.kind = (psp_frame_kind)9;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_frame_serialize(&writer, &input) == PSP_INVALID_VALUE);

    uint8_t truncated[] = {PSP_SIMULATION_VERSION};
    psp_reader_init(&reader, truncated, sizeof(truncated));
    CHECK(psp_frame_deserialize(&reader, &output) == PSP_TRUNCATED_INPUT);

    uint8_t unknown_kind[PSP_FRAME_PREFIX_MAX_SERIALIZED_SIZE] = {
        PSP_SIMULATION_VERSION};
    unknown_kind[PSP_FRAME_PREFIX_MAX_SERIALIZED_SIZE - 1] = 9;
    psp_reader_init(&reader, unknown_kind, sizeof(unknown_kind));
    CHECK(psp_frame_deserialize(&reader, &output) == PSP_INVALID_VALUE);

    psp_writer_init(&writer, bytes, 4);
    input.kind = PSP_FRAME_COMMAND;
    CHECK(psp_frame_serialize(&writer, &input) == PSP_BUFFER_TOO_SMALL);
    return 0;
}
