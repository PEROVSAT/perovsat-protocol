#include "command.h"

psp_status psp_command_serialize(psp_writer *writer, const psp_command *command) {
    if (writer == NULL) return PSP_INVALID_ARGUMENT;
    if (command == NULL) return psp_writer_fail(writer, PSP_INVALID_ARGUMENT);
    psp_write_u8(writer, (uint8_t)command->kind);
    psp_write_u32(writer, command->request_id);
    switch (command->kind) {
    case PSP_COMMAND_UTC_SYNC:
        return psp_command_utc_sync_serialize(writer, &command->body.utc_sync);
    case PSP_COMMAND_GET_STATUS:
        return psp_writer_status(writer);
    default:
        return psp_writer_fail(writer, PSP_INVALID_VALUE);
    }
}

psp_status psp_command_deserialize(psp_reader *reader, psp_command *command) {
    uint8_t kind = 0;
    if (reader == NULL) return PSP_INVALID_ARGUMENT;
    if (command == NULL) return psp_reader_fail(reader, PSP_INVALID_ARGUMENT);
    psp_read_u8(reader, &kind);
    if (psp_reader_status(reader) != PSP_OK) return psp_reader_status(reader);
    if (kind != PSP_COMMAND_UTC_SYNC && kind != PSP_COMMAND_GET_STATUS)
        return psp_reader_fail(reader, PSP_INVALID_VALUE);
    psp_read_u32(reader, &command->request_id);
    if (psp_reader_status(reader) != PSP_OK) return psp_reader_status(reader);
    switch (kind) {
    case PSP_COMMAND_UTC_SYNC:
        command->kind = PSP_COMMAND_UTC_SYNC;
        return psp_command_utc_sync_deserialize(reader, &command->body.utc_sync);
    case PSP_COMMAND_GET_STATUS:
        command->kind = PSP_COMMAND_GET_STATUS;
        return psp_reader_status(reader);
    default:
        return psp_reader_fail(reader, PSP_INVALID_VALUE);
    }
}
