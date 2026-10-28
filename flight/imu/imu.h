#ifndef PSP_FLIGHT_IMU_H
#define PSP_FLIGHT_IMU_H

#include <stdint.h>
#include "../../core/reader.h"
#include "../../core/writer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PSP_SENSOR_VALUE_MAX_SERIALIZED_SIZE 8u
#define PSP_IMU_READING_MAX_SERIALIZED_SIZE 48u

/* Represents whole + micro / 1,000,000 without normalization. */
typedef struct {
    int32_t whole;
    int32_t micro;
} psp_sensor_value;

/* Axis order is x, y, z. Gyroscope units are radians/second; accelerometer
 * units are meters/second squared. */
typedef struct {
    psp_sensor_value gyro[3];
    psp_sensor_value accel[3];
} psp_imu_reading;

psp_status psp_sensor_value_serialize(psp_writer *writer,
                                      const psp_sensor_value *value);
psp_status psp_sensor_value_deserialize(psp_reader *reader,
                                        psp_sensor_value *value);
psp_status psp_imu_reading_serialize(psp_writer *writer,
                                     const psp_imu_reading *reading);
psp_status psp_imu_reading_deserialize(psp_reader *reader,
                                       psp_imu_reading *reading);

#ifdef __cplusplus
}
#endif
#endif
