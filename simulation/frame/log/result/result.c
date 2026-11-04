#include "result.h"

psp_status psp_log_result_serialize(psp_writer *writer,
                                    const psp_log_result *result) {
    if (writer == NULL) return PSP_INVALID_ARGUMENT;
    if (result == NULL) return psp_writer_fail(writer, PSP_INVALID_ARGUMENT);
    if (result->status < PSP_RESULT_APPLIED || result->status > PSP_RESULT_BUSY)
        return psp_writer_fail(writer, PSP_INVALID_VALUE);
    psp_write_u32(writer, result->request_id);
    psp_write_u8(writer, result->command_kind);
    psp_write_u8(writer, result->status);
    return psp_writer_status(writer);
}

psp_status psp_log_result_deserialize(psp_reader *reader,
                                      psp_log_result *result) {
    if (reader == NULL) return PSP_INVALID_ARGUMENT;
    if (result == NULL) return psp_reader_fail(reader, PSP_INVALID_ARGUMENT);
    psp_read_u32(reader, &result->request_id);
    psp_read_u8(reader, &result->command_kind);
    psp_read_u8(reader, &result->status);
    if (psp_reader_status(reader) != PSP_OK) return psp_reader_status(reader);
    if (result->status < PSP_RESULT_APPLIED || result->status > PSP_RESULT_BUSY)
        return psp_reader_fail(reader, PSP_INVALID_VALUE);
    return PSP_OK;
}
