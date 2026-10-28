#ifndef PSP_CORE_READER_H
#define PSP_CORE_READER_H
#include "status.h"
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/*
 * A reader borrows its input buffer for the cursor's lifetime. Initialize a
 * cursor before use; NULL data is valid only when size is zero. Outputs and
 * raw-byte destinations must not overlap the reader input. Primitive reads
 * either produce the complete value and advance position or leave both the
 * output and position unchanged. The first error is sticky, and later
 * operations do nothing. Earlier successful fields are not rolled back when a
 * later operation fails. Position is the number of bytes successfully read or
 * skipped. Call finish only at an outer message boundary; nested codecs must
 * not call it.
 */
typedef struct psp_reader {
  struct {
    const uint8_t *data;
    size_t size;
    size_t position;
    psp_status status;
  } internal;
} psp_reader;

void psp_reader_init(psp_reader *, const uint8_t *, size_t);
psp_status psp_reader_status(const psp_reader *);
psp_status psp_reader_fail(psp_reader *, psp_status);
size_t psp_reader_position(const psp_reader *);
size_t psp_reader_remaining(const psp_reader *);
psp_status psp_reader_finish(psp_reader *);
void psp_read_u8(psp_reader *, uint8_t *);
void psp_read_u16(psp_reader *, uint16_t *);
void psp_read_u32(psp_reader *, uint32_t *);
void psp_read_u64(psp_reader *, uint64_t *);
void psp_read_i8(psp_reader *, int8_t *);
void psp_read_i16(psp_reader *, int16_t *);
void psp_read_i32(psp_reader *, int32_t *);
void psp_read_i64(psp_reader *, int64_t *);
void psp_read_f32(psp_reader *, float *);
void psp_read_bytes(psp_reader *, void *, size_t);
void psp_reader_skip(psp_reader *, size_t);
#ifdef __cplusplus
}
#endif
#endif
