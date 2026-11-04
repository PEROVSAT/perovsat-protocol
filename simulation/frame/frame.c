#include "frame.h"

_Static_assert(PSP_FRAME_PREFIX_MAX_SERIALIZED_SIZE +
                       PSP_COMMAND_MAX_SERIALIZED_SIZE <=
                   PSP_FRAME_MAX_SERIALIZED_SIZE,
               "frame max covers command");
_Static_assert(PSP_FRAME_PREFIX_MAX_SERIALIZED_SIZE +
                       PSP_SAMPLE_MAX_SERIALIZED_SIZE <=
                   PSP_FRAME_MAX_SERIALIZED_SIZE,
               "frame max covers sample");
_Static_assert(PSP_FRAME_PREFIX_MAX_SERIALIZED_SIZE + PSP_LOG_MAX_SERIALIZED_SIZE ==
                   PSP_FRAME_MAX_SERIALIZED_SIZE,
               "frame max is the log arm");
_Static_assert(PSP_SIMULATION_VERSION <= 255u, "frame version is one byte");

int psp_address_accepted(uint8_t destination, uint8_t local) {
    return destination == PSP_ADDRESS_ALL || destination == local;
}

psp_status psp_frame_serialize(psp_writer *writer, const psp_frame *frame) {
    if (writer == NULL) return PSP_INVALID_ARGUMENT;
    if (frame == NULL) return psp_writer_fail(writer, PSP_INVALID_ARGUMENT);
    if (frame->version != PSP_SIMULATION_VERSION)
        return psp_writer_fail(writer, PSP_INVALID_VALUE);
    psp_write_u8(writer, frame->version);
    psp_write_u8(writer, frame->destination);
    psp_write_u8(writer, frame->source);
    psp_write_u64(writer, frame->time_sent);
    psp_write_u64(writer, frame->time_received);
    switch (frame->kind) {
    case PSP_FRAME_COMMAND:
        psp_write_u8(writer, (uint8_t)PSP_FRAME_COMMAND);
        return psp_command_serialize(writer, &frame->body.command);
    case PSP_FRAME_SAMPLE:
        psp_write_u8(writer, (uint8_t)PSP_FRAME_SAMPLE);
        return psp_sample_serialize(writer, &frame->body.sample);
    case PSP_FRAME_LOG:
        psp_write_u8(writer, (uint8_t)PSP_FRAME_LOG);
        return psp_log_serialize(writer, &frame->body.log);
    default:
        return psp_writer_fail(writer, PSP_INVALID_VALUE);
    }
}

psp_status psp_frame_deserialize(psp_reader *reader, psp_frame *frame) {
    uint8_t version = 0;
    uint8_t kind = 0;
    if (reader == NULL) return PSP_INVALID_ARGUMENT;
    if (frame == NULL) return psp_reader_fail(reader, PSP_INVALID_ARGUMENT);
    psp_read_u8(reader, &version);
    if (psp_reader_status(reader) != PSP_OK) return psp_reader_status(reader);
    if (version != PSP_SIMULATION_VERSION)
        return psp_reader_fail(reader, PSP_INVALID_VALUE);
    frame->version = version;
    psp_read_u8(reader, &frame->destination);
    psp_read_u8(reader, &frame->source);
    psp_read_u64(reader, &frame->time_sent);
    psp_read_u64(reader, &frame->time_received);
    psp_read_u8(reader, &kind);
    if (psp_reader_status(reader) != PSP_OK) return psp_reader_status(reader);
    switch (kind) {
    case PSP_FRAME_COMMAND:
        frame->kind = PSP_FRAME_COMMAND;
        return psp_command_deserialize(reader, &frame->body.command);
    case PSP_FRAME_SAMPLE:
        frame->kind = PSP_FRAME_SAMPLE;
        return psp_sample_deserialize(reader, &frame->body.sample);
    case PSP_FRAME_LOG:
        frame->kind = PSP_FRAME_LOG;
        return psp_log_deserialize(reader, &frame->body.log);
    default:
        return psp_reader_fail(reader, PSP_INVALID_VALUE);
    }
}
