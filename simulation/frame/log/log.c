#include "log.h"

_Static_assert(1u + PSP_LOG_UTC_REPORT_MAX_SERIALIZED_SIZE <=
                   PSP_LOG_MAX_SERIALIZED_SIZE,
               "log max covers utc report");

psp_status psp_log_serialize(psp_writer *writer, const psp_log *log) {
    if (writer == NULL) return PSP_INVALID_ARGUMENT;
    if (log == NULL) return psp_writer_fail(writer, PSP_INVALID_ARGUMENT);
    switch (log->kind) {
    case PSP_LOG_UTC_REPORT:
        psp_write_u8(writer, (uint8_t)PSP_LOG_UTC_REPORT);
        return psp_log_utc_report_serialize(writer, &log->body.utc_report);
    case PSP_LOG_I2C_TRANSACTION:
        psp_write_u8(writer, (uint8_t)PSP_LOG_I2C_TRANSACTION);
        return psp_log_i2c_transaction_serialize(writer, &log->body.i2c_transaction);
    case PSP_LOG_RESULT:
        psp_write_u8(writer, (uint8_t)PSP_LOG_RESULT);
        return psp_log_result_serialize(writer, &log->body.result);
    case PSP_LOG_STATUS:
        psp_write_u8(writer, (uint8_t)PSP_LOG_STATUS);
        return psp_log_status_serialize(writer, &log->body.status);
    default:
        return psp_writer_fail(writer, PSP_INVALID_VALUE);
    }
}

psp_status psp_log_deserialize(psp_reader *reader, psp_log *log) {
    uint8_t kind = 0;
    if (reader == NULL) return PSP_INVALID_ARGUMENT;
    if (log == NULL) return psp_reader_fail(reader, PSP_INVALID_ARGUMENT);
    psp_read_u8(reader, &kind);
    if (psp_reader_status(reader) != PSP_OK) return psp_reader_status(reader);
    switch (kind) {
    case PSP_LOG_UTC_REPORT:
        log->kind = PSP_LOG_UTC_REPORT;
        return psp_log_utc_report_deserialize(reader, &log->body.utc_report);
    case PSP_LOG_I2C_TRANSACTION:
        log->kind = PSP_LOG_I2C_TRANSACTION;
        return psp_log_i2c_transaction_deserialize(reader, &log->body.i2c_transaction);
    case PSP_LOG_RESULT:
        log->kind = PSP_LOG_RESULT;
        return psp_log_result_deserialize(reader, &log->body.result);
    case PSP_LOG_STATUS:
        log->kind = PSP_LOG_STATUS;
        return psp_log_status_deserialize(reader, &log->body.status);
    default:
        return psp_reader_fail(reader, PSP_INVALID_VALUE);
    }
}
