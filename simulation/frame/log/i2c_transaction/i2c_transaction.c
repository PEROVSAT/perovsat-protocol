#include "i2c_transaction.h"

#include <string.h>

static int lengths_ok(const psp_log_i2c_transaction *transaction) {
    return transaction->write_length <= PSP_LOG_I2C_TRANSACTION_MAX_WRITE &&
           transaction->read_length <= PSP_LOG_I2C_TRANSACTION_MAX_READ;
}

psp_status psp_log_i2c_transaction_serialize(
    psp_writer *writer, const psp_log_i2c_transaction *transaction) {
    if (writer == NULL) return PSP_INVALID_ARGUMENT;
    if (transaction == NULL)
        return psp_writer_fail(writer, PSP_INVALID_ARGUMENT);
    if (!lengths_ok(transaction))
        return psp_writer_fail(writer, PSP_INVALID_VALUE);

    psp_write_u32(writer, transaction->transaction_id);
    psp_write_u64(writer, transaction->start_utc_ns);
    psp_write_u32(writer, transaction->t_read_offset_us);
    psp_write_u32(writer, transaction->t_end_offset_us);
    psp_write_u16(writer, transaction->flags);
    psp_write_u8(writer, transaction->write_register);
    psp_write_u8(writer, transaction->write_length);
    psp_write_bytes(writer, transaction->write_data, transaction->write_length);
    psp_write_u8(writer, transaction->read_register);
    psp_write_u8(writer, transaction->read_length);
    psp_write_bytes(writer, transaction->read_data, transaction->read_length);
    psp_write_u32(writer, transaction->sample_sequence);
    psp_write_u16(writer, transaction->write_count);
    psp_write_u16(writer, transaction->read_count);
    return psp_writer_status(writer);
}

psp_status psp_log_i2c_transaction_deserialize(
    psp_reader *reader, psp_log_i2c_transaction *transaction) {
    if (reader == NULL) return PSP_INVALID_ARGUMENT;
    if (transaction == NULL)
        return psp_reader_fail(reader, PSP_INVALID_ARGUMENT);
    memset(transaction->write_data, 0, sizeof transaction->write_data);
    memset(transaction->read_data, 0, sizeof transaction->read_data);

    psp_read_u32(reader, &transaction->transaction_id);
    psp_read_u64(reader, &transaction->start_utc_ns);
    psp_read_u32(reader, &transaction->t_read_offset_us);
    psp_read_u32(reader, &transaction->t_end_offset_us);
    psp_read_u16(reader, &transaction->flags);
    psp_read_u8(reader, &transaction->write_register);
    psp_read_u8(reader, &transaction->write_length);
    if (psp_reader_status(reader) != PSP_OK) return psp_reader_status(reader);
    if (transaction->write_length > PSP_LOG_I2C_TRANSACTION_MAX_WRITE)
        return psp_reader_fail(reader, PSP_INVALID_VALUE);

    psp_read_bytes(reader, transaction->write_data, transaction->write_length);
    psp_read_u8(reader, &transaction->read_register);
    psp_read_u8(reader, &transaction->read_length);
    if (psp_reader_status(reader) != PSP_OK) return psp_reader_status(reader);
    if (transaction->read_length > PSP_LOG_I2C_TRANSACTION_MAX_READ)
        return psp_reader_fail(reader, PSP_INVALID_VALUE);

    psp_read_bytes(reader, transaction->read_data, transaction->read_length);
    psp_read_u32(reader, &transaction->sample_sequence);
    psp_read_u16(reader, &transaction->write_count);
    psp_read_u16(reader, &transaction->read_count);
    return psp_reader_status(reader);
}
