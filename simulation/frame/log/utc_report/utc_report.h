#ifndef PSP_SIMULATION_LOG_UTC_REPORT_H
#define PSP_SIMULATION_LOG_UTC_REPORT_H

#include <stdint.h>

#include "../../../../core/reader.h"
#include "../../../../core/writer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PSP_LOG_UTC_REPORT_MAX_SERIALIZED_SIZE (4u + 8u)

/* Reply to one UtcSync. id matches the command. utc_at_receive_ns is this
 * device's UTC estimate at receipt of that command, taken before the
 * command's correction is applied. The report frame's time_sent is the
 * estimate after the correction, at the moment the report is transmitted. */
typedef struct {
    uint32_t id;
    uint64_t utc_at_receive_ns;
} psp_log_utc_report;

psp_status psp_log_utc_report_serialize(psp_writer *writer,
                                        const psp_log_utc_report *report);
psp_status psp_log_utc_report_deserialize(psp_reader *reader,
                                          psp_log_utc_report *report);

#ifdef __cplusplus
}
#endif
#endif
