#include "utc_report.h"

psp_status psp_log_utc_report_serialize(psp_writer *writer,
                                        const psp_log_utc_report *report) {
    if (writer == NULL) return PSP_INVALID_ARGUMENT;
    if (report == NULL) return psp_writer_fail(writer, PSP_INVALID_ARGUMENT);
    psp_write_u32(writer, report->id);
    psp_write_u64(writer, report->utc_at_receive_ns);
    return psp_writer_status(writer);
}

psp_status psp_log_utc_report_deserialize(psp_reader *reader,
                                          psp_log_utc_report *report) {
    if (reader == NULL) return PSP_INVALID_ARGUMENT;
    if (report == NULL) return psp_reader_fail(reader, PSP_INVALID_ARGUMENT);
    psp_read_u32(reader, &report->id);
    psp_read_u64(reader, &report->utc_at_receive_ns);
    return psp_reader_status(reader);
}
