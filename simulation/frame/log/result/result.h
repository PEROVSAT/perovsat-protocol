#ifndef PSP_SIMULATION_LOG_RESULT_H
#define PSP_SIMULATION_LOG_RESULT_H
#include <stdint.h>
#include "../../../../core/reader.h"
#include "../../../../core/writer.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PSP_LOG_RESULT_MAX_SERIALIZED_SIZE 6u
typedef enum psp_result_status {
    PSP_RESULT_APPLIED = 1,
    PSP_RESULT_REJECTED = 2,
    PSP_RESULT_UNSUPPORTED = 3,
    PSP_RESULT_BUSY = 4
} psp_result_status;

typedef struct {
    uint32_t request_id;
    uint8_t command_kind;
    uint8_t status;
} psp_log_result;

psp_status psp_log_result_serialize(psp_writer *writer,
                                    const psp_log_result *result);
psp_status psp_log_result_deserialize(psp_reader *reader,
                                      psp_log_result *result);
#ifdef __cplusplus
}
#endif
#endif
