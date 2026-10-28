#include "imu.h"

psp_status psp_sensor_value_serialize(psp_writer *w, const psp_sensor_value *v) {
    if (w == NULL) return PSP_INVALID_ARGUMENT;
    if (v == NULL) return psp_writer_fail(w, PSP_INVALID_ARGUMENT);
    psp_write_i32(w, v->whole); psp_write_i32(w, v->micro);
    return psp_writer_status(w);
}
psp_status psp_sensor_value_deserialize(psp_reader *r, psp_sensor_value *v) {
    if (r == NULL) return PSP_INVALID_ARGUMENT;
    if (v == NULL) return psp_reader_fail(r, PSP_INVALID_ARGUMENT);
    psp_read_i32(r, &v->whole); psp_read_i32(r, &v->micro);
    return psp_reader_status(r);
}
psp_status psp_imu_reading_serialize(psp_writer *w, const psp_imu_reading *v) {
    if (w == NULL) return PSP_INVALID_ARGUMENT;
    if (v == NULL) return psp_writer_fail(w, PSP_INVALID_ARGUMENT);
    for (unsigned i=0;i<3;i++) psp_sensor_value_serialize(w,&v->gyro[i]);
    for (unsigned i=0;i<3;i++) psp_sensor_value_serialize(w,&v->accel[i]);
    return psp_writer_status(w);
}
psp_status psp_imu_reading_deserialize(psp_reader *r, psp_imu_reading *v) {
    if (r == NULL) return PSP_INVALID_ARGUMENT;
    if (v == NULL) return psp_reader_fail(r, PSP_INVALID_ARGUMENT);
    for (unsigned i=0;i<3;i++) psp_sensor_value_deserialize(r,&v->gyro[i]);
    for (unsigned i=0;i<3;i++) psp_sensor_value_deserialize(r,&v->accel[i]);
    return psp_reader_status(r);
}
