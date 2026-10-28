#include "writer.h"
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
  uint8_t b[40];
  std::memset(b, 0xa5, sizeof b);
  psp_writer w;
  psp_writer_init(&w, b, sizeof b);
  psp_write_u16(&w, 0x1234);
  psp_write_i32(&w, INT32_MIN);
  psp_write_i64(&w, INT64_MAX);
  psp_write_u64(&w, UINT64_C(0x0123456789abcdef));
  float negzero;
  uint32_t nz = 0x80000000u;
  std::memcpy(&negzero, &nz, 4);
  psp_write_f32(&w, negzero);
  const uint8_t expected[] = {0x34, 0x12, 0,    0,    0,    0x80, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f,
                              0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23,
                              0x01, 0,    0,    0,    0x80};
  CHECK(psp_writer_status(&w) == PSP_OK);
  CHECK(psp_writer_position(&w) == sizeof expected);
  CHECK(std::memcmp(b, expected, sizeof expected) == 0);
  CHECK(b[sizeof expected] == 0xa5);
  CHECK(psp_writer_remaining(&w) == sizeof b - sizeof expected);
  psp_writer_zeros(&w, 2);
  CHECK(b[sizeof expected] == 0 && b[sizeof expected + 1] == 0);
  uint32_t nan_bits = 0x7fc12345u;
  float nan_value;
  std::memcpy(&nan_value, &nan_bits, 4);
  psp_writer_init(&w, b, sizeof b);
  psp_write_f32(&w, nan_value);
  const uint8_t nan_expected[] = {0x45, 0x23, 0xc1, 0x7f};
  CHECK(std::memcmp(b, nan_expected, 4) == 0);
  uint8_t data[] = {9, 8};
  psp_write_bytes(&w, data, 2);
  psp_writer_init(&w, nullptr, 0);
  psp_writer_zeros(&w, 0);
  psp_write_bytes(&w, nullptr, 0);
  CHECK(psp_writer_status(&w) == PSP_OK);
  psp_writer_init(&w, b, 1);
  psp_write_u16(&w, 42);
  CHECK(psp_writer_status(&w) == PSP_BUFFER_TOO_SMALL);
  CHECK(psp_writer_position(&w) == 0);
  CHECK(psp_writer_fail(&w, PSP_INVALID_VALUE) == PSP_BUFFER_TOO_SMALL);
  psp_write_u8(&w, 1);
  CHECK(psp_writer_position(&w) == 0);
  psp_writer_init(&w, b, 4);
  psp_write_bytes(&w, nullptr, 1);
  CHECK(psp_writer_status(&w) == PSP_INVALID_ARGUMENT &&
        psp_writer_position(&w) == 0);
  for (size_t width : {size_t(1), size_t(2), size_t(4), size_t(8)}) {
    std::memset(b, 0xa5, sizeof b);
    psp_writer_init(&w, b, width - 1);
    if (width == 1)
      psp_write_u8(&w, 0x12);
    if (width == 2)
      psp_write_u16(&w, 0x1234);
    if (width == 4)
      psp_write_u32(&w, 0x12345678u);
    if (width == 8)
      psp_write_u64(&w, UINT64_C(0x123456789abcdef0));
    CHECK(psp_writer_status(&w) == PSP_BUFFER_TOO_SMALL &&
          psp_writer_position(&w) == 0);
    CHECK(b[0] == 0xa5 && b[sizeof b - 1] == 0xa5);
  }
  psp_writer_init(&w, b, 3);
  psp_write_f32(&w, 1.0f);
  CHECK(psp_writer_status(&w) == PSP_BUFFER_TOO_SMALL &&
        psp_writer_position(&w) == 0 && b[0] == 0xa5);
  psp_writer_init(&w, b, 3);
  psp_writer_zeros(&w, 4);
  CHECK(psp_writer_status(&w) == PSP_BUFFER_TOO_SMALL &&
        psp_writer_position(&w) == 0 && b[0] == 0xa5);
  psp_writer_init(&w, b, 3);
  psp_write_bytes(&w, data, 4);
  CHECK(psp_writer_status(&w) == PSP_BUFFER_TOO_SMALL &&
        psp_writer_position(&w) == 0 && b[0] == 0xa5);
  psp_writer_init(&w, b, 3);
  CHECK(psp_writer_fail(&w, PSP_INVALID_VALUE) == PSP_INVALID_VALUE);
  psp_write_u8(&w, 1);
  CHECK(psp_writer_position(&w) == 0 && b[0] == 0xa5);
  psp_writer_init(&w, nullptr, 1);
  CHECK(psp_writer_status(&w) == PSP_INVALID_ARGUMENT);
  psp_writer_init(&w, b, 2);
  psp_writer_zeros(&w, 0);
  CHECK(psp_writer_status(&w) == PSP_OK && psp_writer_position(&w) == 0);
  psp_write_u8(nullptr, 1);
  CHECK(psp_writer_status(nullptr) == PSP_INVALID_ARGUMENT);
  CHECK(psp_writer_fail(nullptr, PSP_INVALID_VALUE) == PSP_INVALID_ARGUMENT);
  return 0;
}
