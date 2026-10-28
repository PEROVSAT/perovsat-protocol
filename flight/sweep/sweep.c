#include "sweep.h"

#define WRITE_META_FLOAT(field) psp_write_f32(w, v->field)
#define READ_META_FLOAT(field) psp_read_f32(r, &v->field)

psp_status psp_sweep_metadata_serialize(psp_writer *w, const psp_sweep_metadata *v) {
    if (w == NULL) return PSP_INVALID_ARGUMENT;
    if (v == NULL) return psp_writer_fail(w, PSP_INVALID_ARGUMENT);
    WRITE_META_FLOAT(voc); WRITE_META_FLOAT(isc);
    WRITE_META_FLOAT(tsensor_start); WRITE_META_FLOAT(tsensor_end);
    WRITE_META_FLOAT(ff); WRITE_META_FLOAT(eff); WRITE_META_FLOAT(vmax);
    WRITE_META_FLOAT(imax); WRITE_META_FLOAT(pmax); WRITE_META_FLOAT(adc);
    psp_write_u32(w, v->timestamp); psp_write_u32(w, v->crc);
    return psp_writer_status(w);
}
psp_status psp_sweep_metadata_deserialize(psp_reader *r, psp_sweep_metadata *v) {
    if (r == NULL) return PSP_INVALID_ARGUMENT;
    if (v == NULL) return psp_reader_fail(r, PSP_INVALID_ARGUMENT);
    READ_META_FLOAT(voc); READ_META_FLOAT(isc);
    READ_META_FLOAT(tsensor_start); READ_META_FLOAT(tsensor_end);
    READ_META_FLOAT(ff); READ_META_FLOAT(eff); READ_META_FLOAT(vmax);
    READ_META_FLOAT(imax); READ_META_FLOAT(pmax); READ_META_FLOAT(adc);
    psp_read_u32(r, &v->timestamp); psp_read_u32(r, &v->crc);
    return psp_reader_status(r);
}
#undef WRITE_META_FLOAT
#undef READ_META_FLOAT
psp_status psp_sweep_points_serialize(psp_writer *w, const psp_sweep_points *v) {
    if (w == NULL) return PSP_INVALID_ARGUMENT;
    if (v == NULL) return psp_writer_fail(w, PSP_INVALID_ARGUMENT);
    for (unsigned i=0;i<PSP_IV_POINTS;i++) psp_write_f32(w,v->voltage[i]);
    for (unsigned i=0;i<PSP_IV_POINTS;i++) psp_write_f32(w,v->current[i]);
    return psp_writer_status(w);
}
psp_status psp_sweep_points_deserialize(psp_reader *r, psp_sweep_points *v) {
    if (r == NULL) return PSP_INVALID_ARGUMENT;
    if (v == NULL) return psp_reader_fail(r, PSP_INVALID_ARGUMENT);
    for (unsigned i=0;i<PSP_IV_POINTS;i++) psp_read_f32(r,&v->voltage[i]);
    for (unsigned i=0;i<PSP_IV_POINTS;i++) psp_read_f32(r,&v->current[i]);
    return psp_reader_status(r);
}
psp_status psp_sweep_serialize(psp_writer *w, const psp_sweep *v) {
    if (w == NULL) return PSP_INVALID_ARGUMENT;
    if (v == NULL) return psp_writer_fail(w, PSP_INVALID_ARGUMENT);
    psp_sweep_metadata_serialize(w,&v->meta); psp_sweep_points_serialize(w,&v->iv);
    return psp_writer_status(w);
}
psp_status psp_sweep_deserialize(psp_reader *r, psp_sweep *v) {
    if (r == NULL) return PSP_INVALID_ARGUMENT;
    if (v == NULL) return psp_reader_fail(r, PSP_INVALID_ARGUMENT);
    psp_sweep_metadata_deserialize(r,&v->meta); psp_sweep_points_deserialize(r,&v->iv);
    return psp_reader_status(r);
}
