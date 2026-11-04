#ifndef PSP_SIMULATION_FRAME_H
#define PSP_SIMULATION_FRAME_H

#include <stdint.h>

#include "../../core/reader.h"
#include "../../core/writer.h"
#include "../version.h"
#include "command/command.h"
#include "log/log.h"
#include "sample/sample.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PSP_FRAME_PREFIX_MAX_SERIALIZED_SIZE (1u + 1u + 1u + 8u + 8u + 1u)
/* The log arm is the largest body. Command and sample must fit inside it. */
#define PSP_FRAME_MAX_SERIALIZED_SIZE \
    (PSP_FRAME_PREFIX_MAX_SERIALIZED_SIZE + PSP_LOG_MAX_SERIALIZED_SIZE)

/* General call. Every participant accepts it. It is not a participant address. */
#define PSP_ADDRESS_ALL ((uint8_t)0)

typedef enum psp_frame_kind {
    PSP_FRAME_COMMAND = 1,
    PSP_FRAME_SAMPLE = 2,
    PSP_FRAME_LOG = 3
} psp_frame_kind;

/* One record on the bench link. version must be PSP_SIMULATION_VERSION;
 * serialization and deserialization reject any other value. The transport
 * delivers every frame to every participant. A receiver accepts a frame whose
 * destination is PSP_ADDRESS_ALL or equal to its own address, and ignores the
 * rest.
 *
 * source is the transmitting participant address; the codec does not impose
 * nonzero addressing. time_sent and time_received are nanoseconds since the
 * Unix epoch on the sender's and receiver's UTC estimates. A device's estimate
 * starts as its free-running counter and becomes a rough UTC once corrected
 * it through UtcSync. The sender writes time_sent and writes time_received
 * as 0. The receiving endpoint may record time_received in its own copy after
 * a successful decode. A bridge forwards the encoded bytes and does not write
 * either field.
 *
 * Command, sample, and log are variants. Command bodies begin with a command
 * kind and request_id. Their arms carry the remaining payload.
 * *_MAX_SERIALIZED_SIZE is the buffer capacity, not the length of every value.
 * psp_writer_position is the encoded length. Nested codecs do not call
 * psp_reader_finish; the caller does that at this outer boundary. */
typedef struct {
    uint8_t version;
    uint8_t destination;
    uint8_t source;
    uint64_t time_sent;
    uint64_t time_received;
    psp_frame_kind kind;
    union {
        psp_command command;
        psp_sample sample;
        psp_log log;
    } body;
} psp_frame;

/* Returns 1 when destination is the general call or equals local. */
int psp_address_accepted(uint8_t destination, uint8_t local);

psp_status psp_frame_serialize(psp_writer *writer, const psp_frame *frame);
psp_status psp_frame_deserialize(psp_reader *reader, psp_frame *frame);

#ifdef __cplusplus
}
#endif
#endif
