#ifndef PSP_FLIGHT_PAYLOAD_READING_H
#define PSP_FLIGHT_PAYLOAD_READING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../imu/imu.h"
#include "../sweep/sweep.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PSP_PAYLOAD_MAX_AMUS 31u
#define PSP_PAYLOAD_READING_MAX_SERIALIZED_SIZE \
    (4u + PSP_IMU_READING_MAX_SERIALIZED_SIZE + \
     PSP_PAYLOAD_MAX_AMUS * PSP_SWEEP_MAX_SERIALIZED_SIZE)
#define PSP_PAYLOAD_READING_MAX_SERIALIZED_SIZE_FOR(capacity) \
    ((size_t)4u + (size_t)PSP_IMU_READING_MAX_SERIALIZED_SIZE + \
     (size_t)(capacity) * (size_t)PSP_SWEEP_MAX_SERIALIZED_SIZE)

typedef struct {
    uint8_t amu_id; /* Stable identity 0..30, independent of storage position. */
    psp_sweep sweep;
} psp_indexed_sweep;

/* Storage is borrowed from the caller and must outlive this view. The active
 * entries occupy [0, sweep_count); inactive entries need not be initialized.
 * Keep the struct internal state coherent by using the helpers below. */
typedef struct {
    struct {
        bool imu_present;
        psp_imu_reading imu;
        uint8_t sweep_count;
        uint8_t sweep_capacity;
        psp_indexed_sweep *sweeps;
    } internal;
} psp_payload_reading;

/* capacity is storage slots, not the maximum stable AMU ID. Zero capacity with
 * NULL storage is valid. Positive capacity requires caller-owned storage. */
psp_status psp_payload_reading_init(psp_payload_reading *reading,
                                    psp_indexed_sweep *storage,
                                    size_t capacity);
/* Clears presence and active count while preserving storage and capacity. */
psp_status psp_payload_reading_clear(psp_payload_reading *reading);
psp_status psp_payload_reading_set_sweep(psp_payload_reading *reading,
                                         uint8_t stable_id,
                                         const psp_sweep *sweep);
/* Missing IDs are a successful no-op. Removal may move the last active entry;
 * pointers returned by getters are invalidated by any removal or clear. */
psp_status psp_payload_reading_clear_sweep(psp_payload_reading *reading,
                                           uint8_t stable_id);
/* Returns NULL when absent, invalid, or stable_id is outside 0..30. */
const psp_sweep *psp_payload_reading_get_sweep(
    const psp_payload_reading *reading, uint8_t stable_id);
psp_status psp_payload_reading_set_imu(psp_payload_reading *reading,
                                       const psp_imu_reading *imu);
psp_status psp_payload_reading_clear_imu(psp_payload_reading *reading);
/* Returns NULL when IMU data is absent or the reading state is invalid. */
const psp_imu_reading *psp_payload_reading_get_imu(
    const psp_payload_reading *reading);
size_t psp_payload_reading_sweep_count(const psp_payload_reading *reading);

/* Body: little-endian u32 mask, optional 48-byte IMU, then present sweeps in
 * ascending stable ID order. Mask bit 0 is IMU; bits 1..31 select AMU IDs 0..30.
 * Capacity is not encoded. Decode consumes this body's fields only; callers at
 * an outer boundary should call psp_reader_finish. On decode error, output can
 * be partially updated and must be discarded. */
psp_status psp_payload_reading_serialize(psp_writer *writer,
                                         const psp_payload_reading *reading);
psp_status psp_payload_reading_deserialize(psp_reader *reader,
                                           psp_payload_reading *reading);

#ifdef __cplusplus
}
#endif
#endif
