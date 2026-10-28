#include "writer.h"
#include <float.h>
#include <limits.h>
#include <string.h>

_Static_assert(CHAR_BIT == 8, "PSP requires 8-bit bytes");
_Static_assert(sizeof(float) == 4 && FLT_RADIX == 2 && FLT_MANT_DIG == 24 &&
                   FLT_MAX_EXP == 128,
               "PSP requires IEEE-754 binary32 float");

void psp_writer_init(psp_writer *w, uint8_t *data, size_t capacity) {
  if (!w)
    return;
  w->internal.data = data;
  w->internal.capacity = capacity;
  w->internal.position = 0;
  w->internal.status = (data || capacity == 0) ? PSP_OK : PSP_INVALID_ARGUMENT;
}
psp_status psp_writer_status(const psp_writer *w) {
  return w ? w->internal.status : PSP_INVALID_ARGUMENT;
}
psp_status psp_writer_fail(psp_writer *w, psp_status s) {
  if (!w)
    return PSP_INVALID_ARGUMENT;
  if (w->internal.status == PSP_OK && s != PSP_OK)
    w->internal.status = s;
  return w->internal.status;
}
size_t psp_writer_position(const psp_writer *w) {
  return w ? w->internal.position : 0;
}
size_t psp_writer_remaining(const psp_writer *w) {
  return w && w->internal.position <= w->internal.capacity
             ? w->internal.capacity - w->internal.position
             : 0;
}
static int reserve(psp_writer *w, size_t n) {
  if (!w || w->internal.status != PSP_OK)
    return 0;
  if (n > psp_writer_remaining(w)) {
    psp_writer_fail(w, PSP_BUFFER_TOO_SMALL);
    return 0;
  }
  return 1;
}
void psp_write_bytes(psp_writer *w, const void *src, size_t n) {
  if (!w || w->internal.status != PSP_OK)
    return;
  if (n && !src) {
    psp_writer_fail(w, PSP_INVALID_ARGUMENT);
    return;
  }
  if (!reserve(w, n))
    return;
  if (n)
    memcpy(w->internal.data + w->internal.position, src, n);
  w->internal.position += n;
}
void psp_writer_zeros(psp_writer *w, size_t n) {
  if (!reserve(w, n))
    return;
  if (n)
    memset(w->internal.data + w->internal.position, 0, n);
  w->internal.position += n;
}
static void write_le(psp_writer *w, uint64_t v, size_t n) {
  uint8_t b[8];
  size_t i;
  for (i = 0; i < n; i++)
    b[i] = (uint8_t)(v >> (8u * i));
  psp_write_bytes(w, b, n);
}
void psp_write_u8(psp_writer *w, uint8_t v) { write_le(w, v, 1); }
void psp_write_u16(psp_writer *w, uint16_t v) { write_le(w, v, 2); }
void psp_write_u32(psp_writer *w, uint32_t v) { write_le(w, v, 4); }
void psp_write_u64(psp_writer *w, uint64_t v) { write_le(w, v, 8); }
void psp_write_i8(psp_writer *w, int8_t v) { write_le(w, (uint8_t)v, 1); }
void psp_write_i16(psp_writer *w, int16_t v) { write_le(w, (uint16_t)v, 2); }
void psp_write_i32(psp_writer *w, int32_t v) { write_le(w, (uint32_t)v, 4); }
void psp_write_i64(psp_writer *w, int64_t v) { write_le(w, (uint64_t)v, 8); }
void psp_write_f32(psp_writer *w, float v) {
  uint32_t bits;
  memcpy(&bits, &v, sizeof bits);
  write_le(w, bits, 4);
}
