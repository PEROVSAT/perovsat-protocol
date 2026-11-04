#include "utc_sync.h"

psp_status psp_command_utc_sync_serialize(psp_writer *writer,
                                          const psp_command_utc_sync *command) {
    if (writer == NULL) return PSP_INVALID_ARGUMENT;
    if (command == NULL) return psp_writer_fail(writer, PSP_INVALID_ARGUMENT);
    psp_write_i64(writer, command->offset_correction_ns);
    psp_write_i32(writer, command->rate_ppb);
    return psp_writer_status(writer);
}

psp_status psp_command_utc_sync_deserialize(psp_reader *reader,
                                            psp_command_utc_sync *command) {
    if (reader == NULL) return PSP_INVALID_ARGUMENT;
    if (command == NULL) return psp_reader_fail(reader, PSP_INVALID_ARGUMENT);
    psp_read_i64(reader, &command->offset_correction_ns);
    psp_read_i32(reader, &command->rate_ppb);
    return psp_reader_status(reader);
}
