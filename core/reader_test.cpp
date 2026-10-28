#include "reader.h"
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <limits>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #x);        \
      return 1;                                                                \
    }                                                                          \
  } while (0)
int main() {
  const uint8_t b[] = {0x34, 0x12, 0,    0,    0,    0x80, 0xff, 0xff, 0xff,
                       0xff, 0xff, 0xff, 0xff, 0x7f, 0xef, 0xcd, 0xab, 0x89,
                       0x67, 0x45, 0x23, 0x01, 0,    0,    0,    0x80};
  psp_reader r;
  psp_reader_init(&r, b, sizeof b);
  uint16_t u16 = 0;
  int32_t i32 = 0;
  int64_t i64 = 0;
  uint64_t u64 = 0;
  float f = 1;
  psp_read_u16(&r, &u16);
  psp_read_i32(&r, &i32);
  psp_read_i64(&r, &i64);
  psp_read_u64(&r, &u64);
  psp_read_f32(&r, &f);
  uint32_t bits = 0;
  std::memcpy(&bits, &f, 4);
  CHECK(u16 == 0x1234 && i32 == INT32_MIN && i64 == INT64_MAX &&
        u64 == UINT64_C(0x0123456789abcdef) && bits == 0x80000000u);
  CHECK(psp_reader_status(&r) == PSP_OK &&
        psp_reader_position(&r) == sizeof b && psp_reader_remaining(&r) == 0);
  CHECK(psp_reader_finish(&r) == PSP_OK);
  uint8_t one = 0;
  psp_reader_init(&r, b, 1);
  uint16_t sentinel = 0xbeef;
  psp_read_u16(&r, &sentinel);
  CHECK(sentinel == 0xbeef && psp_reader_position(&r) == 0 &&
        psp_reader_status(&r) == PSP_TRUNCATED_INPUT);
  CHECK(psp_reader_fail(&r, PSP_INVALID_VALUE) == PSP_TRUNCATED_INPUT);
  psp_read_u8(&r, &one);
  CHECK(one == 0 && psp_reader_position(&r) == 0);
  psp_reader_init(&r, b, sizeof b);
  psp_reader_skip(&r, 5);
  CHECK(psp_reader_position(&r) == 5);
  uint8_t out[2] = {0};
  psp_read_bytes(&r, out, 2);
  CHECK(out[0] == 0x80 && out[1] == 0xff);
  psp_reader_init(&r, b, 2);
  psp_reader_skip(&r, 3);
  CHECK(psp_reader_status(&r) == PSP_TRUNCATED_INPUT &&
        psp_reader_position(&r) == 0);
  psp_reader_init(&r, b, sizeof b);
  CHECK(psp_reader_finish(&r) == PSP_TRAILING_DATA);
  CHECK(psp_reader_finish(&r) == PSP_TRAILING_DATA);
  const uint8_t nan_bytes[] = {0x45, 0x23, 0xc1, 0x7f};
  psp_reader_init(&r, nan_bytes, 4);
  f = 0;
  psp_read_f32(&r, &f);
  bits = 0;
  std::memcpy(&bits, &f, 4);
  CHECK(bits == 0x7fc12345u);
  psp_reader_init(&r, nullptr, 0);
  psp_reader_skip(&r, 0);
  psp_read_bytes(&r, nullptr, 0);
  CHECK(psp_reader_finish(&r) == PSP_OK);
  psp_reader_init(&r, b, sizeof b);
  psp_read_bytes(&r, nullptr, 1);
  CHECK(psp_reader_status(&r) == PSP_INVALID_ARGUMENT &&
        psp_reader_position(&r) == 0);
  psp_reader_init(&r, b, sizeof b);
  psp_read_u64(&r, nullptr);
  CHECK(psp_reader_status(&r) == PSP_INVALID_ARGUMENT &&
        psp_reader_position(&r) == 0);
  const uint8_t endpoints[] = {0x80, 0x7f, 0x00, 0x80, 0xff, 0x7f, 0x00, 0x00,
                               0x00, 0x80, 0xff, 0xff, 0xff, 0x7f, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0xff,
                               0xff, 0xff, 0xff, 0xff, 0xff, 0x7f};
  psp_reader_init(&r, endpoints, sizeof endpoints);
  int8_t i8a = 0, i8b = 0;
  int16_t i16a = 0, i16b = 0;
  int32_t i32a = 0, i32b = 0;
  int64_t i64a = 0, i64b = 0;
  psp_read_i8(&r, &i8a);
  psp_read_i8(&r, &i8b);
  psp_read_i16(&r, &i16a);
  psp_read_i16(&r, &i16b);
  psp_read_i32(&r, &i32a);
  psp_read_i32(&r, &i32b);
  psp_read_i64(&r, &i64a);
  psp_read_i64(&r, &i64b);
  CHECK(i8a == INT8_MIN && i8b == INT8_MAX && i16a == INT16_MIN &&
        i16b == INT16_MAX);
  CHECK(i32a == INT32_MIN && i32b == INT32_MAX && i64a == INT64_MIN &&
        i64b == INT64_MAX);
  const uint8_t infs[] = {0, 0, 0x80, 0x7f, 0, 0, 0x80, 0xff};
  psp_reader_init(&r, infs, sizeof infs);
  float fp = 0, fn = 0;
  psp_read_f32(&r, &fp);
  psp_read_f32(&r, &fn);
  std::memcpy(&bits, &fp, 4);
  CHECK(bits == 0x7f800000u);
  std::memcpy(&bits, &fn, 4);
  CHECK(bits == 0xff800000u);
  /* Every primitive width must fail atomically when one byte short. */
  const uint8_t eight[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  for (size_t available = 0; available < 8; ++available) {
    psp_reader_init(&r, eight, available);
    uint64_t sentinel64 = UINT64_C(0xfeedfacecafebeef);
    psp_read_u64(&r, &sentinel64);
    CHECK(sentinel64 == UINT64_C(0xfeedfacecafebeef));
    CHECK(psp_reader_position(&r) == 0 &&
          psp_reader_status(&r) == PSP_TRUNCATED_INPUT);
  }
  for (size_t width : {size_t(1), size_t(2), size_t(4)}) {
    psp_reader_init(&r, eight, width - 1);
    uint32_t sentinel32 = 0xdeadbeefu;
    if (width == 1) {
      uint8_t v = 0x5a;
      psp_read_u8(&r, &v);
      CHECK(v == 0x5a);
    }
    if (width == 2) {
      uint16_t v = 0x5a5a;
      psp_read_u16(&r, &v);
      CHECK(v == 0x5a5a);
    }
    if (width == 4) {
      psp_read_u32(&r, &sentinel32);
      CHECK(sentinel32 == 0xdeadbeefu);
    }
    CHECK(psp_reader_position(&r) == 0 &&
          psp_reader_status(&r) == PSP_TRUNCATED_INPUT);
  }
  psp_reader_init(&r, eight, 3);
  float float_sentinel = 12.5f;
  psp_read_f32(&r, &float_sentinel);
  CHECK(float_sentinel == 12.5f && psp_reader_position(&r) == 0 &&
        psp_reader_status(&r) == PSP_TRUNCATED_INPUT);
  psp_reader_init(&r, eight, 8);
  CHECK(psp_reader_fail(&r, PSP_INVALID_VALUE) == PSP_INVALID_VALUE);
  psp_read_u8(&r, &one);
  CHECK(one == 0 && psp_reader_position(&r) == 0);
  CHECK(psp_reader_finish(&r) == PSP_INVALID_VALUE);
  psp_reader_init(&r, nullptr, 1);
  CHECK(psp_reader_status(&r) == PSP_INVALID_ARGUMENT);
  psp_reader_skip(&r, 1);
  CHECK(psp_reader_position(&r) == 0);
  psp_reader_init(nullptr, b, 1);
  psp_read_u8(nullptr, &one);
  CHECK(psp_reader_status(nullptr) == PSP_INVALID_ARGUMENT);
  CHECK(psp_reader_position(nullptr) == 0);
  return 0;
}
