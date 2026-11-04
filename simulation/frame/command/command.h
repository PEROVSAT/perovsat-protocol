#ifndef PSP_SIMULATION_COMMAND_H
#define PSP_SIMULATION_COMMAND_H

#include <stdint.h>

#include "utc_sync/utc_sync.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PSP_COMMAND_MAX_SERIALIZED_SIZE \
    (1u + 4u + PSP_COMMAND_UTC_SYNC_MAX_SERIALIZED_SIZE)

typedef enum psp_command_kind {
    PSP_COMMAND_UTC_SYNC = 1,
    PSP_COMMAND_GET_STATUS = 2
} psp_command_kind;

/* Variant over command arms. Address and timestamps live on the frame. */
typedef struct {
    psp_command_kind kind;
    uint32_t request_id;
    union {
        psp_command_utc_sync utc_sync;
    } body;
} psp_command;

psp_status psp_command_serialize(psp_writer *writer, const psp_command *command);
psp_status psp_command_deserialize(psp_reader *reader, psp_command *command);

#ifdef __cplusplus
}
#endif
#endif
