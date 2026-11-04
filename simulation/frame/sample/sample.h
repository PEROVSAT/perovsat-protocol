#ifndef PSP_SIMULATION_SAMPLE_H
#define PSP_SIMULATION_SAMPLE_H

#include <stdint.h>

#include "imu_stimulus/imu_stimulus.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PSP_SAMPLE_MAX_SERIALIZED_SIZE \
    (1u + PSP_SAMPLE_IMU_STIMULUS_MAX_SERIALIZED_SIZE)

typedef enum psp_sample_kind {
    PSP_SAMPLE_IMU_STIMULUS = 1
} psp_sample_kind;

/* Variant over sample arms. Address and timestamps live on the frame. */
typedef struct {
    psp_sample_kind kind;
    union {
        psp_sample_imu_stimulus imu_stimulus;
    } body;
} psp_sample;

psp_status psp_sample_serialize(psp_writer *writer, const psp_sample *sample);
psp_status psp_sample_deserialize(psp_reader *reader, psp_sample *sample);

#ifdef __cplusplus
}
#endif
#endif
