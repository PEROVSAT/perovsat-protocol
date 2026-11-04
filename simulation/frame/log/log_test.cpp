#include "log.h"

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
    uint8_t bytes[PSP_LOG_MAX_SERIALIZED_SIZE];
    psp_writer writer;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_log_serialize(&writer, nullptr) == PSP_INVALID_ARGUMENT);

    psp_log utc{};
    utc.kind = PSP_LOG_UTC_REPORT;
    utc.body.utc_report.id = 9;
    utc.body.utc_report.utc_at_receive_ns = 5000;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_log_serialize(&writer, &utc) == PSP_OK);
    psp_log output{};
    psp_reader reader;
    psp_reader_init(&reader, bytes, psp_writer_position(&writer));
    CHECK(psp_log_deserialize(&reader, &output) == PSP_OK);
    CHECK(psp_reader_finish(&reader) == PSP_OK);
    CHECK(output.kind == PSP_LOG_UTC_REPORT);
    CHECK(output.body.utc_report.id == 9);
    CHECK(output.body.utc_report.utc_at_receive_ns == 5000);

    psp_log access{};
    access.kind = PSP_LOG_I2C_TRANSACTION;
    access.body.i2c_transaction.t_end_offset_us = 400;
    access.body.i2c_transaction.read_length = 1;
    access.body.i2c_transaction.read_data[0] = 0x68;
    access.body.i2c_transaction.sample_sequence = 7;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_log_serialize(&writer, &access) == PSP_OK);
    psp_reader_init(&reader, bytes, psp_writer_position(&writer));
    CHECK(psp_log_deserialize(&reader, &output) == PSP_OK);
    CHECK(psp_reader_finish(&reader) == PSP_OK);
    CHECK(output.kind == PSP_LOG_I2C_TRANSACTION);
    CHECK(output.body.i2c_transaction.t_end_offset_us == 400);
    CHECK(output.body.i2c_transaction.read_data[0] == 0x68);
    CHECK(output.body.i2c_transaction.sample_sequence == 7);

    psp_log result{};
    result.kind = PSP_LOG_RESULT;
    result.body.result.request_id = 11;
    result.body.result.command_kind = 2;
    result.body.result.status = PSP_RESULT_APPLIED;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_log_serialize(&writer, &result) == PSP_OK);
    psp_reader_init(&reader, bytes, psp_writer_position(&writer));
    CHECK(psp_log_deserialize(&reader, &output) == PSP_OK);
    CHECK(psp_reader_finish(&reader) == PSP_OK);
    CHECK(output.kind == PSP_LOG_RESULT);
    CHECK(output.body.result.request_id == 11);
    CHECK(output.body.result.command_kind == 2);
    CHECK(output.body.result.status == PSP_RESULT_APPLIED);

    psp_log status{};
    status.kind = PSP_LOG_STATUS;
    status.body.status.flags = PSP_STATUS_FLAG_SAMPLE_PRESENT |
                               PSP_STATUS_FLAG_CLOCK_CORRECTED;
    status.body.status.active_sample_sequence = 7;
    status.body.status.transactions = 8;
    status.body.status.dropped_transactions = 9;
    status.body.status.rejected_samples = 10;
    status.body.status.rx_errors = 11;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_log_serialize(&writer, &status) == PSP_OK);
    psp_reader_init(&reader, bytes, psp_writer_position(&writer));
    CHECK(psp_log_deserialize(&reader, &output) == PSP_OK);
    CHECK(psp_reader_finish(&reader) == PSP_OK);
    CHECK(output.kind == PSP_LOG_STATUS);
    CHECK(output.body.status.flags == 3);
    CHECK(output.body.status.active_sample_sequence == 7);
    CHECK(output.body.status.rx_errors == 11);

    utc.kind = (psp_log_kind)9;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_log_serialize(&writer, &utc) == PSP_INVALID_VALUE);
    uint8_t unknown[] = {9};
    psp_reader_init(&reader, unknown, sizeof(unknown));
    CHECK(psp_log_deserialize(&reader, &output) == PSP_INVALID_VALUE);
    return 0;
}
