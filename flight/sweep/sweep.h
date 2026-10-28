#ifndef PSP_FLIGHT_SWEEP_H
#define PSP_FLIGHT_SWEEP_H

#include "../../core/reader.h"
#include "../../core/writer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PSP_IV_POINTS 40u
#define PSP_SWEEP_METADATA_MAX_SERIALIZED_SIZE 48u
#define PSP_SWEEP_POINTS_MAX_SERIALIZED_SIZE 320u
#define PSP_SWEEP_MAX_SERIALIZED_SIZE 368u

/* Driver-provided sweep values, serialized in this field order. crc is carried
 * as source metadata and is not computed or treated as a protocol checksum. */
typedef struct {
    float voc;
    float isc;
    float tsensor_start;
    float tsensor_end;
    float ff;
    float eff;
    float vmax;
    float imax;
    float pmax;
    float adc;
    uint32_t timestamp;
    uint32_t crc;
} psp_sweep_metadata;

typedef struct {
    float voltage[PSP_IV_POINTS];
    float current[PSP_IV_POINTS];
} psp_sweep_points;

typedef struct {
    psp_sweep_metadata meta;
    psp_sweep_points iv;
} psp_sweep;

psp_status psp_sweep_metadata_serialize(psp_writer *writer,
                                         const psp_sweep_metadata *metadata);
psp_status psp_sweep_metadata_deserialize(psp_reader *reader,
                                           psp_sweep_metadata *metadata);
psp_status psp_sweep_points_serialize(psp_writer *writer,
                                      const psp_sweep_points *points);
psp_status psp_sweep_points_deserialize(psp_reader *reader,
                                        psp_sweep_points *points);
psp_status psp_sweep_serialize(psp_writer *writer, const psp_sweep *sweep);
psp_status psp_sweep_deserialize(psp_reader *reader, psp_sweep *sweep);

#ifdef __cplusplus
}
#endif
#endif
