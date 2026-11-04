#ifndef PSP_SIMULATION_LOG_I2C_TRANSACTION_H
#define PSP_SIMULATION_LOG_I2C_TRANSACTION_H

#include <stdint.h>

#include "../../../../core/reader.h"
#include "../../../../core/writer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PSP_LOG_I2C_TRANSACTION_MAX_WRITE 16u
#define PSP_LOG_I2C_TRANSACTION_MAX_READ 16u
#define PSP_LOG_I2C_TRANSACTION_NO_READ 0xffffffffu
#define PSP_LOG_I2C_TRANSACTION_NO_SAMPLE 0xffffffffu
#define PSP_LOG_I2C_TRANSACTION_FLAG_ABORTED 1u
#define PSP_LOG_I2C_TRANSACTION_FLAG_WRITE_TRUNCATED 2u
#define PSP_LOG_I2C_TRANSACTION_FLAG_READ_TRUNCATED 4u
#define PSP_LOG_I2C_TRANSACTION_FLAG_TIME_OVERFLOW 8u
#define PSP_LOG_I2C_TRANSACTION_FLAG_COUNT_OVERFLOW 16u
#define PSP_LOG_I2C_TRANSACTION_FLAG_UNSUPPORTED 32u
#define PSP_LOG_I2C_TRANSACTION_FLAG_SAMPLE_SATURATED 64u
#define PSP_LOG_I2C_TRANSACTION_FLAG_UTC_UNSYNCED 128u
#define PSP_LOG_I2C_TRANSACTION_MAX_SERIALIZED_SIZE            \
    (4u + 8u + 4u + 4u + 2u + 1u + 1u + PSP_LOG_I2C_TRANSACTION_MAX_WRITE + \
     1u + 1u + PSP_LOG_I2C_TRANSACTION_MAX_READ + 4u + 2u + 2u)

/* One I2C transaction record. start_utc_ns is the UTC estimate at transaction
 * start; the enclosing frame time_sent is serialization/transmit time. Offset
 * fields are monotonic microseconds after transaction start.
 * t_read_offset_us is PSP_LOG_I2C_TRANSACTION_NO_READ when the transaction had
 * no read. Bytes beyond write_length and read_length are not encoded.
 * sample_sequence is the stimulus sequence being served, or
 * PSP_LOG_I2C_TRANSACTION_NO_SAMPLE when there was none. Counts are total
 * actual data bytes excluding register pointers; captures may be truncated.
 * Flags are PSP_LOG_I2C_TRANSACTION_FLAG_* bits.
 * Lengths above the array bounds are rejected. */
typedef struct {
    uint32_t transaction_id;
    uint64_t start_utc_ns;
    uint32_t t_read_offset_us;
    uint32_t t_end_offset_us;
    uint16_t flags;
    uint8_t write_register;
    uint8_t write_length;
    uint8_t write_data[PSP_LOG_I2C_TRANSACTION_MAX_WRITE];
    uint8_t read_register;
    uint8_t read_length;
    uint8_t read_data[PSP_LOG_I2C_TRANSACTION_MAX_READ];
    uint32_t sample_sequence;
    uint16_t write_count;
    uint16_t read_count;
} psp_log_i2c_transaction;

psp_status psp_log_i2c_transaction_serialize(
    psp_writer *writer, const psp_log_i2c_transaction *transaction);
psp_status psp_log_i2c_transaction_deserialize(
    psp_reader *reader, psp_log_i2c_transaction *transaction);

#ifdef __cplusplus
}
#endif
#endif
