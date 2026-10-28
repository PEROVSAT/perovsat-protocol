#ifndef PSP_CORE_WRITER_H
#define PSP_CORE_WRITER_H
#include "status.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/*
 * A writer borrows its output buffer for the cursor's lifetime. Initialize a
 * cursor before use; NULL data is valid only when capacity is zero. The caller
 * must not overlap raw-byte source data with this buffer or otherwise access
 * the buffer concurrently. Primitive writes either write the complete value
 * and advance position or leave both buffer and position unchanged. The first
 * error is sticky, and later operations do nothing. Earlier successful fields
 * are not rolled back when a later operation fails.
 */
typedef struct psp_writer {
  struct {
    uint8_t *data;
    size_t capacity;
    size_t position;
    psp_status status;
  } internal;
} psp_writer;

void psp_writer_init(psp_writer *, uint8_t *, size_t);
psp_status psp_writer_status(const psp_writer *);
psp_status psp_writer_fail(psp_writer *, psp_status);
size_t psp_writer_position(const psp_writer *);
size_t psp_writer_remaining(const psp_writer *);
void psp_write_u8(psp_writer *, uint8_t);
void psp_write_u16(psp_writer *, uint16_t);
void psp_write_u32(psp_writer *, uint32_t);
void psp_write_u64(psp_writer *, uint64_t);
void psp_write_i8(psp_writer *, int8_t);
void psp_write_i16(psp_writer *, int16_t);
void psp_write_i32(psp_writer *, int32_t);
void psp_write_i64(psp_writer *, int64_t);
void psp_write_f32(psp_writer *, float);
/* A NULL source is valid only for count zero. */
void psp_write_bytes(psp_writer *, const void *, size_t);
void psp_writer_zeros(psp_writer *, size_t);
#ifdef __cplusplus
}
#endif
#endif
