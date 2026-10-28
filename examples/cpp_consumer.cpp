#include <cstdint>

#include "core/reader.h"
#include "core/writer.h"
#include "flight/payload_reading/payload_reading.h"

int main() {
    static psp_indexed_sweep sender_storage[1];
    static psp_indexed_sweep ground_storage[PSP_PAYLOAD_MAX_AMUS];
    static std::uint8_t bytes[PSP_PAYLOAD_READING_MAX_SERIALIZED_SIZE];
    psp_payload_reading sender{};
    psp_payload_reading ground{};

    psp_sweep sweep{};
    sweep.meta.voc = 3.4F;
    if (psp_payload_reading_init(&sender, sender_storage, 1) != PSP_OK ||
        psp_payload_reading_set_sweep(&sender, 30, &sweep) != PSP_OK ||
        psp_payload_reading_init(&ground, ground_storage,
                                 PSP_PAYLOAD_MAX_AMUS) != PSP_OK) {
        return 1;
    }

    psp_writer writer{};
    psp_writer_init(&writer, bytes, sizeof(bytes));
    if (psp_payload_reading_serialize(&writer, &sender) != PSP_OK) {
        return 2;
    }

    psp_reader reader{};
    psp_reader_init(&reader, bytes, psp_writer_position(&writer));
    if (psp_payload_reading_deserialize(&reader, &ground) != PSP_OK ||
        psp_reader_finish(&reader) != PSP_OK) {
        return 3;
    }
    const psp_sweep *decoded = psp_payload_reading_get_sweep(&ground, 30);
    return decoded != nullptr && decoded->meta.voc == 3.4F ? 0 : 4;
}
