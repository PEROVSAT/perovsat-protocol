#ifndef PSP_SIMULATION_COMMAND_UTC_SYNC_H
#define PSP_SIMULATION_COMMAND_UTC_SYNC_H

#include <stdint.h>

#include "../../../../core/reader.h"
#include "../../../../core/writer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PSP_COMMAND_UTC_SYNC_MAX_SERIALIZED_SIZE (8u + 4u)

/* One time-sync probe from the bridge. The enclosing command request_id is
 * copied into the UtcReport.
 * The command frame's time_sent is the host's UTC, in nanoseconds since the
 * Unix epoch, at transmit.
 *
 * The device stamps its report from the UTC estimate it already has, then
 * applies this command. offset_correction_ns is added to that estimate.
 * rate_ppb replaces the estimate's rate relative to the device's free-running
 * counter, in parts per billion. Positive rate_ppb makes the estimate advance
 * faster than the counter. A zero correction and a repeated rate ask for a new
 * measurement and leave the running estimate's offset unchanged. */
typedef struct {
    int64_t offset_correction_ns;
    int32_t rate_ppb;
} psp_command_utc_sync;

psp_status psp_command_utc_sync_serialize(psp_writer *writer,
                                          const psp_command_utc_sync *command);
psp_status psp_command_utc_sync_deserialize(psp_reader *reader,
                                            psp_command_utc_sync *command);

#ifdef __cplusplus
}
#endif
#endif
