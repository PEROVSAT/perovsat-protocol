#include <stdint.h>
#include "core/reader.h"
#include "core/writer.h"
#include "flight/payload_reading/payload_reading.h"

int main(void) {
    static psp_indexed_sweep sender_storage[1];
    static psp_indexed_sweep receiver_storage[PSP_PAYLOAD_MAX_AMUS];
    psp_payload_reading sender;
    psp_payload_reading receiver;
    uint8_t bytes[PSP_PAYLOAD_READING_MAX_SERIALIZED_SIZE_FOR(1)];
    psp_writer writer;
    psp_reader reader;

    psp_sweep sweep = {0};
    sweep.meta.voc = 1.25f;
    sweep.iv.voltage[0] = 0.5f;
    if (psp_payload_reading_init(&sender, sender_storage, 1) != PSP_OK ||
        psp_payload_reading_set_sweep(&sender, 2, &sweep) != PSP_OK ||
        psp_payload_reading_init(&receiver, receiver_storage,
                                 PSP_PAYLOAD_MAX_AMUS) != PSP_OK) {
        return 1;
    }

    psp_writer_init(&writer, bytes, sizeof(bytes));
    if (psp_payload_reading_serialize(&writer, &sender) != PSP_OK ||
        psp_writer_status(&writer) != PSP_OK ||
        psp_writer_position(&writer) != 372) {
        return 2;
    }

    const size_t encoded_size = psp_writer_position(&writer);
    psp_reader_init(&reader, bytes, encoded_size);
    if (psp_payload_reading_deserialize(&reader, &receiver) != PSP_OK ||
        psp_reader_finish(&reader) != PSP_OK) {
        return 3;
    }
    const psp_sweep *decoded = psp_payload_reading_get_sweep(&receiver, 2);
    return decoded != 0 && decoded->meta.voc == 1.25f &&
                   decoded->iv.voltage[0] == 0.5f
               ? 0
               : 4;
}
