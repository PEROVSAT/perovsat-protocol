#include "sample.h"

psp_status psp_sample_serialize(psp_writer *writer, const psp_sample *sample) {
    if (writer == NULL) return PSP_INVALID_ARGUMENT;
    if (sample == NULL) return psp_writer_fail(writer, PSP_INVALID_ARGUMENT);
    switch (sample->kind) {
    case PSP_SAMPLE_IMU_STIMULUS:
        psp_write_u8(writer, (uint8_t)PSP_SAMPLE_IMU_STIMULUS);
        return psp_sample_imu_stimulus_serialize(writer, &sample->body.imu_stimulus);
    default:
        return psp_writer_fail(writer, PSP_INVALID_VALUE);
    }
}

psp_status psp_sample_deserialize(psp_reader *reader, psp_sample *sample) {
    uint8_t kind = 0;
    if (reader == NULL) return PSP_INVALID_ARGUMENT;
    if (sample == NULL) return psp_reader_fail(reader, PSP_INVALID_ARGUMENT);
    psp_read_u8(reader, &kind);
    if (psp_reader_status(reader) != PSP_OK) return psp_reader_status(reader);
    switch (kind) {
    case PSP_SAMPLE_IMU_STIMULUS:
        sample->kind = PSP_SAMPLE_IMU_STIMULUS;
        return psp_sample_imu_stimulus_deserialize(reader, &sample->body.imu_stimulus);
    default:
        return psp_reader_fail(reader, PSP_INVALID_VALUE);
    }
}
