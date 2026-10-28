#include "reader.h"

#include <float.h>
#include <limits.h>
#include <string.h>

_Static_assert(CHAR_BIT == 8, "PSP requires 8-bit bytes");
_Static_assert(sizeof(float) == 4 && FLT_RADIX == 2 && FLT_MANT_DIG == 24 &&
                   FLT_MAX_EXP == 128,
               "PSP requires IEEE-754 binary32 float");

void psp_reader_init(psp_reader *reader, const uint8_t *data, size_t size) {
  if (reader == NULL) {
    return;
  }

  reader->internal.data = data;
  reader->internal.size = size;
  reader->internal.position = 0;
  reader->internal.status =
      (data != NULL || size == 0) ? PSP_OK : PSP_INVALID_ARGUMENT;
}

psp_status psp_reader_status(const psp_reader *reader) {
  return reader != NULL ? reader->internal.status : PSP_INVALID_ARGUMENT;
}

psp_status psp_reader_fail(psp_reader *reader, psp_status status) {
  if (reader == NULL) {
    return PSP_INVALID_ARGUMENT;
  }
  if (reader->internal.status == PSP_OK && status != PSP_OK) {
    reader->internal.status = status;
  }
  return reader->internal.status;
}

size_t psp_reader_position(const psp_reader *reader) {
  return reader != NULL ? reader->internal.position : 0;
}

size_t psp_reader_remaining(const psp_reader *reader) {
  if (reader == NULL || reader->internal.position > reader->internal.size) {
    return 0;
  }
  return reader->internal.size - reader->internal.position;
}

psp_status psp_reader_finish(psp_reader *reader) {
  if (reader == NULL) {
    return PSP_INVALID_ARGUMENT;
  }
  if (reader->internal.status != PSP_OK) {
    return reader->internal.status;
  }
  if (psp_reader_remaining(reader) != 0) {
    return psp_reader_fail(reader, PSP_TRAILING_DATA);
  }
  return PSP_OK;
}

static int reader_reserve(psp_reader *reader, size_t count) {
  if (reader == NULL || reader->internal.status != PSP_OK) {
    return 0;
  }
  if (count > psp_reader_remaining(reader)) {
    psp_reader_fail(reader, PSP_TRUNCATED_INPUT);
    return 0;
  }
  return 1;
}

void psp_read_bytes(psp_reader *reader, void *destination, size_t count) {
  if (reader == NULL || reader->internal.status != PSP_OK) {
    return;
  }
  if (count != 0 && destination == NULL) {
    psp_reader_fail(reader, PSP_INVALID_ARGUMENT);
    return;
  }
  if (!reader_reserve(reader, count)) {
    return;
  }
  if (count != 0) {
    memcpy(destination, reader->internal.data + reader->internal.position,
           count);
  }
  reader->internal.position += count;
}

void psp_reader_skip(psp_reader *reader, size_t count) {
  if (!reader_reserve(reader, count)) {
    return;
  }
  reader->internal.position += count;
}

static uint64_t read_le(psp_reader *reader, size_t width) {
  uint64_t value = 0;
  size_t index;

  if (!reader_reserve(reader, width)) {
    return 0;
  }
  for (index = 0; index < width; ++index) {
    value |= (uint64_t)reader->internal.data[reader->internal.position + index]
             << (8u * index);
  }
  reader->internal.position += width;
  return value;
}

static int64_t signed_value(uint64_t bits, unsigned width) {
  const uint64_t sign = UINT64_C(1) << (width - 1);
  uint64_t mask;
  uint64_t magnitude;

  if ((bits & sign) == 0) {
    return (int64_t)bits;
  }
  mask = width == 64 ? UINT64_MAX : (UINT64_C(1) << width) - 1;
  magnitude = ((~bits) & mask) + 1;
  if (width == 64 && magnitude == sign) {
    return INT64_MIN;
  }
  return -(int64_t)magnitude;
}

#define DEFINE_READ_UNSIGNED(name, type, width)                                \
  void name(psp_reader *reader, type *output) {                                \
    uint64_t value;                                                            \
    if (reader == NULL || reader->internal.status != PSP_OK)                   \
      return;                                                                  \
    if (output == NULL) {                                                      \
      psp_reader_fail(reader, PSP_INVALID_ARGUMENT);                           \
      return;                                                                  \
    }                                                                          \
    value = read_le(reader, width);                                            \
    if (reader->internal.status == PSP_OK)                                     \
      *output = (type)value;                                                   \
  }

DEFINE_READ_UNSIGNED(psp_read_u8, uint8_t, 1)
DEFINE_READ_UNSIGNED(psp_read_u16, uint16_t, 2)
DEFINE_READ_UNSIGNED(psp_read_u32, uint32_t, 4)
DEFINE_READ_UNSIGNED(psp_read_u64, uint64_t, 8)

#define DEFINE_READ_SIGNED(name, type, width)                                  \
  void name(psp_reader *reader, type *output) {                                \
    uint64_t bits;                                                             \
    if (reader == NULL || reader->internal.status != PSP_OK)                   \
      return;                                                                  \
    if (output == NULL) {                                                      \
      psp_reader_fail(reader, PSP_INVALID_ARGUMENT);                           \
      return;                                                                  \
    }                                                                          \
    bits = read_le(reader, width);                                             \
    if (reader->internal.status == PSP_OK)                                     \
      *output = (type)signed_value(bits, width * 8u);                          \
  }

DEFINE_READ_SIGNED(psp_read_i8, int8_t, 1)
DEFINE_READ_SIGNED(psp_read_i16, int16_t, 2)
DEFINE_READ_SIGNED(psp_read_i32, int32_t, 4)
DEFINE_READ_SIGNED(psp_read_i64, int64_t, 8)

void psp_read_f32(psp_reader *reader, float *output) {
  uint32_t bits;

  if (reader == NULL || reader->internal.status != PSP_OK) {
    return;
  }
  if (output == NULL) {
    psp_reader_fail(reader, PSP_INVALID_ARGUMENT);
    return;
  }
  bits = (uint32_t)read_le(reader, 4);
  if (reader->internal.status == PSP_OK) {
    memcpy(output, &bits, sizeof(bits));
  }
}
