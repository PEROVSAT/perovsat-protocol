#ifndef PSP_SIMULATION_LOG_STATUS_H
#define PSP_SIMULATION_LOG_STATUS_H
#include <stdint.h>
#include "../../../../core/reader.h"
#include "../../../../core/writer.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PSP_LOG_STATUS_MAX_SERIALIZED_SIZE 22u
#define PSP_STATUS_FLAG_SAMPLE_PRESENT 1u
#define PSP_STATUS_FLAG_CLOCK_CORRECTED 2u
typedef struct {
    uint16_t flags;
    uint32_t active_sample_sequence;
    uint32_t transactions;
    uint32_t dropped_transactions;
    uint32_t rejected_samples;
    uint32_t rx_errors;
} psp_log_status;

psp_status psp_log_status_serialize(psp_writer *writer,
                                    const psp_log_status *status);
psp_status psp_log_status_deserialize(psp_reader *reader,
                                      psp_log_status *status);
#ifdef __cplusplus
}
#endif
#endif
