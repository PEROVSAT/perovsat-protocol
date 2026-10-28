#ifndef PSP_CORE_STATUS_H
#define PSP_CORE_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum psp_status {
  PSP_OK = 0,
  PSP_INVALID_ARGUMENT,
  PSP_BUFFER_TOO_SMALL,
  PSP_TRUNCATED_INPUT,
  PSP_INVALID_VALUE,
  PSP_CAPACITY_EXCEEDED,
  PSP_TRAILING_DATA
} psp_status;

#ifdef __cplusplus
}
#endif
#endif
