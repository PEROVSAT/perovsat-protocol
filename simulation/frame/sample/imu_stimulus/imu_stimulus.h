#ifndef PSP_SIMULATION_SAMPLE_IMU_STIMULUS_H
#define PSP_SIMULATION_SAMPLE_IMU_STIMULUS_H

#include <stdint.h>

#include "../../../../core/reader.h"
#include "../../../../core/writer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* sequence, three specific-force axes, three angular-rate axes, presence, and
 * an optional temperature. */
#define PSP_SAMPLE_IMU_STIMULUS_MAX_SERIALIZED_SIZE \
    (4u + 12u + 12u + 1u + 4u)

/* Body-frame stimulus for an emulated IMU. Specific force is meters per second
 * squared. Angular rate is radians per second. sequence is the producer's
 * counter; an access log echoes it as the sample that was served.
 * temperature_present is 0 or 1. temperature_c is degrees Celsius and is
 * encoded only when temperature_present is 1. */
typedef struct {
    uint32_t sequence;
    float accel_m_s2[3];
    float gyro_rad_s[3];
    uint8_t temperature_present;
    float temperature_c;
} psp_sample_imu_stimulus;

psp_status psp_sample_imu_stimulus_serialize(
    psp_writer *writer, const psp_sample_imu_stimulus *sample);
psp_status psp_sample_imu_stimulus_deserialize(
    psp_reader *reader, psp_sample_imu_stimulus *sample);

#ifdef __cplusplus
}
#endif
#endif
