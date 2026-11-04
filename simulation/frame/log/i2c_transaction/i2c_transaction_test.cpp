#include "i2c_transaction.h"

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
    uint8_t bytes[PSP_LOG_I2C_TRANSACTION_MAX_SERIALIZED_SIZE];
    psp_writer writer;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_log_i2c_transaction_serialize(&writer, nullptr) ==
          PSP_INVALID_ARGUMENT);

    psp_log_i2c_transaction input{};
    input.t_read_offset_us = 80;
    input.t_end_offset_us = 400;
    input.flags = 0x1;
    input.write_register = 0x3b;
    input.write_length = 0;
    input.read_register = 0x3b;
    input.read_length = 14;
    for (uint8_t i = 0; i < input.read_length; ++i)
        input.read_data[i] = (uint8_t)(0xa0 + i);
    input.sample_sequence = 7;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_log_i2c_transaction_serialize(&writer, &input) == PSP_OK);

    psp_log_i2c_transaction output{};
    psp_reader reader;
    psp_reader_init(&reader, bytes, psp_writer_position(&writer));
    CHECK(psp_log_i2c_transaction_deserialize(&reader, &output) == PSP_OK);
    CHECK(psp_reader_finish(&reader) == PSP_OK);
    CHECK(output.t_read_offset_us == 80);
    CHECK(output.t_end_offset_us == 400);
    CHECK(output.flags == 0x1);
    CHECK(output.write_length == 0);
    CHECK(output.read_register == 0x3b && output.read_length == 14);
    CHECK(output.read_data[0] == 0xa0 && output.read_data[13] == 0xad);
    CHECK(output.read_data[14] == 0);
    CHECK(output.sample_sequence == 7);

    input.t_read_offset_us = PSP_LOG_I2C_TRANSACTION_NO_READ;
    input.write_length = 1;
    input.write_data[0] = 0x19;
    input.read_length = 0;
    input.sample_sequence = PSP_LOG_I2C_TRANSACTION_NO_SAMPLE;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_log_i2c_transaction_serialize(&writer, &input) == PSP_OK);
    psp_reader_init(&reader, bytes, psp_writer_position(&writer));
    CHECK(psp_log_i2c_transaction_deserialize(&reader, &output) == PSP_OK);
    CHECK(psp_reader_finish(&reader) == PSP_OK);
    CHECK(output.t_read_offset_us == PSP_LOG_I2C_TRANSACTION_NO_READ);
    CHECK(output.write_data[0] == 0x19);
    CHECK(output.sample_sequence == PSP_LOG_I2C_TRANSACTION_NO_SAMPLE);

    input.write_length = PSP_LOG_I2C_TRANSACTION_MAX_WRITE;
    input.read_length = PSP_LOG_I2C_TRANSACTION_MAX_READ;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_log_i2c_transaction_serialize(&writer, &input) == PSP_OK);
    CHECK(psp_writer_position(&writer) == PSP_LOG_I2C_TRANSACTION_MAX_SERIALIZED_SIZE);
    psp_reader_init(&reader, bytes, psp_writer_position(&writer));
    CHECK(psp_log_i2c_transaction_deserialize(&reader, &output) == PSP_OK);
    CHECK(psp_reader_finish(&reader) == PSP_OK);
    CHECK(output.write_length == PSP_LOG_I2C_TRANSACTION_MAX_WRITE);
    CHECK(output.read_length == PSP_LOG_I2C_TRANSACTION_MAX_READ);

    input.write_length = (uint8_t)(PSP_LOG_I2C_TRANSACTION_MAX_WRITE + 1u);
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_log_i2c_transaction_serialize(&writer, &input) == PSP_INVALID_VALUE);

    uint8_t oversize[24] = {};
    oversize[23] = 17;
    psp_reader_init(&reader, oversize, sizeof(oversize));
    CHECK(psp_log_i2c_transaction_deserialize(&reader, &output) == PSP_INVALID_VALUE);
    return 0;
}
