#include "imu_stimulus.h"

psp_status psp_sample_imu_stimulus_serialize(
    psp_writer *writer, const psp_sample_imu_stimulus *sample) {
    unsigned axis;
    if (writer == NULL) return PSP_INVALID_ARGUMENT;
    if (sample == NULL) return psp_writer_fail(writer, PSP_INVALID_ARGUMENT);
    if (sample->sequence == 0xffffffffu)
        return psp_writer_fail(writer, PSP_INVALID_VALUE);
    if (sample->temperature_present > 1u)
        return psp_writer_fail(writer, PSP_INVALID_VALUE);
    psp_write_u32(writer, sample->sequence);
    for (axis = 0; axis < 3u; ++axis)
        psp_write_f32(writer, sample->accel_m_s2[axis]);
    for (axis = 0; axis < 3u; ++axis)
        psp_write_f32(writer, sample->gyro_rad_s[axis]);
    psp_write_u8(writer, sample->temperature_present);
    if (sample->temperature_present)
        psp_write_f32(writer, sample->temperature_c);
    return psp_writer_status(writer);
}

psp_status psp_sample_imu_stimulus_deserialize(
    psp_reader *reader, psp_sample_imu_stimulus *sample) {
    unsigned axis;
    if (reader == NULL) return PSP_INVALID_ARGUMENT;
    if (sample == NULL) return psp_reader_fail(reader, PSP_INVALID_ARGUMENT);
    psp_read_u32(reader, &sample->sequence);
    if (psp_reader_status(reader) != PSP_OK) return psp_reader_status(reader);
    if (sample->sequence == 0xffffffffu)
        return psp_reader_fail(reader, PSP_INVALID_VALUE);
    for (axis = 0; axis < 3u; ++axis)
        psp_read_f32(reader, &sample->accel_m_s2[axis]);
    for (axis = 0; axis < 3u; ++axis)
        psp_read_f32(reader, &sample->gyro_rad_s[axis]);
    psp_read_u8(reader, &sample->temperature_present);
    if (psp_reader_status(reader) != PSP_OK) return psp_reader_status(reader);
    if (sample->temperature_present > 1u)
        return psp_reader_fail(reader, PSP_INVALID_VALUE);
    sample->temperature_c = 0.f;
    if (sample->temperature_present)
        psp_read_f32(reader, &sample->temperature_c);
    return psp_reader_status(reader);
}
