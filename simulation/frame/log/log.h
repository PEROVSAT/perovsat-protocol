#ifndef PSP_SIMULATION_LOG_H
#define PSP_SIMULATION_LOG_H

#include <stdint.h>

#include "i2c_transaction/i2c_transaction.h"
#include "utc_report/utc_report.h"
#include "result/result.h"
#include "status/status.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The I2C transaction is the largest arm. UtcReport must fit inside it. */
#define PSP_LOG_MAX_SERIALIZED_SIZE \
    (1u + PSP_LOG_I2C_TRANSACTION_MAX_SERIALIZED_SIZE)

typedef enum psp_log_kind {
    PSP_LOG_UTC_REPORT = 1,
    PSP_LOG_I2C_TRANSACTION = 2,
    PSP_LOG_RESULT = 3,
    PSP_LOG_STATUS = 4
} psp_log_kind;

/* Variant over log arms. Address and timestamps live on the frame. */
typedef struct {
    psp_log_kind kind;
    union {
        psp_log_utc_report utc_report;
        psp_log_i2c_transaction i2c_transaction;
        psp_log_result result;
        psp_log_status status;
    } body;
} psp_log;

psp_status psp_log_serialize(psp_writer *writer, const psp_log *log);
psp_status psp_log_deserialize(psp_reader *reader, psp_log *log);

#ifdef __cplusplus
}
#endif
#endif
