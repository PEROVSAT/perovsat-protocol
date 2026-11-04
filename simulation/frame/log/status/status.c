#include "status.h"

psp_status psp_log_status_serialize(psp_writer *writer,
                                    const psp_log_status *status) {
    if (writer == NULL) return PSP_INVALID_ARGUMENT;
    if (status == NULL) return psp_writer_fail(writer, PSP_INVALID_ARGUMENT);
    psp_write_u16(writer, status->flags);
    psp_write_u32(writer, status->active_sample_sequence);
    psp_write_u32(writer, status->transactions);
    psp_write_u32(writer, status->dropped_transactions);
    psp_write_u32(writer, status->rejected_samples);
    psp_write_u32(writer, status->rx_errors);
    return psp_writer_status(writer);
}

psp_status psp_log_status_deserialize(psp_reader *reader,
                                      psp_log_status *status) {
    if (reader == NULL) return PSP_INVALID_ARGUMENT;
    if (status == NULL) return psp_reader_fail(reader, PSP_INVALID_ARGUMENT);
    psp_read_u16(reader, &status->flags);
    psp_read_u32(reader, &status->active_sample_sequence);
    psp_read_u32(reader, &status->transactions);
    psp_read_u32(reader, &status->dropped_transactions);
    psp_read_u32(reader, &status->rejected_samples);
    psp_read_u32(reader, &status->rx_errors);
    return psp_reader_status(reader);
}
