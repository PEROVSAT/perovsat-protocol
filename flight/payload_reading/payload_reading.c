#include "payload_reading.h"

static psp_status invalid_reading(psp_payload_reading *reading) {
    (void)reading;
    return PSP_INVALID_VALUE;
}

static int is_valid(const psp_payload_reading *reading) {
    if (reading == NULL || reading->internal.sweep_capacity > PSP_PAYLOAD_MAX_AMUS ||
        reading->internal.sweep_count > reading->internal.sweep_capacity ||
        (reading->internal.sweep_capacity != 0 && reading->internal.sweeps == NULL)) {
        return 0;
    }
    for (size_t i = 0; i < reading->internal.sweep_count; ++i) {
        if (reading->internal.sweeps[i].amu_id >= PSP_PAYLOAD_MAX_AMUS) return 0;
        for (size_t j = 0; j < i; ++j) {
            if (reading->internal.sweeps[j].amu_id == reading->internal.sweeps[i].amu_id) return 0;
        }
    }
    return 1;
}

psp_status psp_payload_reading_init(psp_payload_reading *reading,
                                    psp_indexed_sweep *storage, size_t capacity) {
    if (reading == NULL || capacity > PSP_PAYLOAD_MAX_AMUS ||
        (capacity != 0 && storage == NULL)) return PSP_INVALID_ARGUMENT;
    reading->internal.imu_present = false;
    reading->internal.sweep_count = 0;
    reading->internal.sweep_capacity = (uint8_t)capacity;
    reading->internal.sweeps = storage;
    return PSP_OK;
}

psp_status psp_payload_reading_clear(psp_payload_reading *reading) {
    if (reading == NULL) return PSP_INVALID_ARGUMENT;
    if (!is_valid(reading)) return invalid_reading(reading);
    reading->internal.imu_present = false;
    reading->internal.sweep_count = 0;
    return PSP_OK;
}

psp_status psp_payload_reading_set_sweep(psp_payload_reading *reading,
                                         uint8_t stable_id,
                                         const psp_sweep *sweep) {
    if (reading == NULL || sweep == NULL || stable_id >= PSP_PAYLOAD_MAX_AMUS)
        return PSP_INVALID_ARGUMENT;
    if (!is_valid(reading)) return invalid_reading(reading);
    size_t index = 0;
    while (index < reading->internal.sweep_count &&
           reading->internal.sweeps[index].amu_id != stable_id) ++index;
    if (index == reading->internal.sweep_count && index == reading->internal.sweep_capacity)
        return PSP_CAPACITY_EXCEEDED;
    reading->internal.sweeps[index].amu_id = stable_id;
    reading->internal.sweeps[index].sweep = *sweep;
    if (index == reading->internal.sweep_count) ++reading->internal.sweep_count;
    return PSP_OK;
}

psp_status psp_payload_reading_clear_sweep(psp_payload_reading *reading,
                                           uint8_t stable_id) {
    if (reading == NULL || stable_id >= PSP_PAYLOAD_MAX_AMUS) return PSP_INVALID_ARGUMENT;
    if (!is_valid(reading)) return invalid_reading(reading);
    for (size_t i = 0; i < reading->internal.sweep_count; ++i) {
        if (reading->internal.sweeps[i].amu_id == stable_id) {
            --reading->internal.sweep_count;
            reading->internal.sweeps[i] = reading->internal.sweeps[reading->internal.sweep_count];
            return PSP_OK;
        }
    }
    return PSP_OK;
}

const psp_sweep *psp_payload_reading_get_sweep(const psp_payload_reading *reading,
                                               uint8_t stable_id) {
    if (!is_valid(reading) || stable_id >= PSP_PAYLOAD_MAX_AMUS) return NULL;
    for (size_t i = 0; i < reading->internal.sweep_count; ++i)
        if (reading->internal.sweeps[i].amu_id == stable_id) return &reading->internal.sweeps[i].sweep;
    return NULL;
}

psp_status psp_payload_reading_set_imu(psp_payload_reading *reading,
                                       const psp_imu_reading *imu) {
    if (reading == NULL || imu == NULL) return PSP_INVALID_ARGUMENT;
    if (!is_valid(reading)) return invalid_reading(reading);
    reading->internal.imu = *imu;
    reading->internal.imu_present = true;
    return PSP_OK;
}

psp_status psp_payload_reading_clear_imu(psp_payload_reading *reading) {
    if (reading == NULL) return PSP_INVALID_ARGUMENT;
    if (!is_valid(reading)) return invalid_reading(reading);
    reading->internal.imu_present = false;
    return PSP_OK;
}

const psp_imu_reading *psp_payload_reading_get_imu(const psp_payload_reading *reading) {
    return is_valid(reading) && reading->internal.imu_present ? &reading->internal.imu : NULL;
}

size_t psp_payload_reading_sweep_count(const psp_payload_reading *reading) {
    return is_valid(reading) ? reading->internal.sweep_count : 0;
}

psp_status psp_payload_reading_serialize(psp_writer *writer,
                                         const psp_payload_reading *reading) {
    if (writer == NULL) return PSP_INVALID_ARGUMENT;
    if (reading == NULL) return psp_writer_fail(writer, PSP_INVALID_ARGUMENT);
    if (!is_valid(reading)) return psp_writer_fail(writer, PSP_INVALID_VALUE);
    uint32_t mask = reading->internal.imu_present ? 1u : 0u;
    for (size_t i = 0; i < reading->internal.sweep_count; ++i)
        mask |= UINT32_C(1) << (reading->internal.sweeps[i].amu_id + 1u);
    psp_write_u32(writer, mask);
    if (reading->internal.imu_present) psp_imu_reading_serialize(writer, &reading->internal.imu);
    for (uint8_t id = 0; id < PSP_PAYLOAD_MAX_AMUS; ++id) {
        for (size_t i = 0; i < reading->internal.sweep_count; ++i) {
            if (reading->internal.sweeps[i].amu_id == id)
                psp_sweep_serialize(writer, &reading->internal.sweeps[i].sweep);
        }
    }
    return psp_writer_status(writer);
}

psp_status psp_payload_reading_deserialize(psp_reader *reader,
                                           psp_payload_reading *reading) {
    if (reader == NULL) return PSP_INVALID_ARGUMENT;
    if (reading == NULL) return psp_reader_fail(reader, PSP_INVALID_ARGUMENT);
    if (!is_valid(reading)) return psp_reader_fail(reader, PSP_INVALID_VALUE);
    uint32_t mask = 0;
    psp_read_u32(reader, &mask);
    if (psp_reader_status(reader) != PSP_OK) return psp_reader_status(reader);
    size_t count = 0;
    for (unsigned id = 0; id < PSP_PAYLOAD_MAX_AMUS; ++id)
        count += (mask >> (id + 1u)) & 1u;
    if (count > reading->internal.sweep_capacity)
        return psp_reader_fail(reader, PSP_CAPACITY_EXCEEDED);

    reading->internal.imu_present = (mask & 1u) != 0;
    if (reading->internal.imu_present) psp_imu_reading_deserialize(reader, &reading->internal.imu);
    reading->internal.sweep_count = 0;
    for (uint8_t id = 0; id < PSP_PAYLOAD_MAX_AMUS; ++id) {
        if ((mask & (UINT32_C(1) << (id + 1u))) != 0) {
            size_t index = reading->internal.sweep_count++;
            reading->internal.sweeps[index].amu_id = id;
            psp_sweep_deserialize(reader, &reading->internal.sweeps[index].sweep);
            if (psp_reader_status(reader) != PSP_OK) return psp_reader_status(reader);
        }
    }
    return psp_reader_status(reader);
}
