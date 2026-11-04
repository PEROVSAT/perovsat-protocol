#include "utc_report.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#define CHECK(condition)                                                       \
    do {                                                                       \
        if (!(condition)) {                                                    \
            std::fprintf(stderr, "failed line %d: %s\n", __LINE__,             \
                         #condition);                                          \
            std::exit(1);                                                      \
        }                                                                      \
    } while (false)

int main() {
    uint8_t bytes[PSP_LOG_UTC_REPORT_MAX_SERIALIZED_SIZE];
    psp_writer writer;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_log_utc_report_serialize(&writer, nullptr) == PSP_INVALID_ARGUMENT);

    psp_log_utc_report input{};
    input.id = 9;
    input.utc_at_receive_ns = 0x0807060504030201ull;
    psp_writer_init(&writer, bytes, sizeof(bytes));
    CHECK(psp_log_utc_report_serialize(&writer, &input) == PSP_OK);
    CHECK(psp_writer_position(&writer) == PSP_LOG_UTC_REPORT_MAX_SERIALIZED_SIZE);

    psp_log_utc_report output{};
    psp_reader reader;
    psp_reader_init(&reader, bytes, psp_writer_position(&writer));
    CHECK(psp_log_utc_report_deserialize(&reader, &output) == PSP_OK);
    CHECK(psp_reader_finish(&reader) == PSP_OK);
    CHECK(output.id == 9);
    CHECK(output.utc_at_receive_ns == 0x0807060504030201ull);

    psp_reader_init(&reader, bytes, 3);
    CHECK(psp_log_utc_report_deserialize(&reader, &output) == PSP_TRUNCATED_INPUT);
    return 0;
}
