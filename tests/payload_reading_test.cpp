#include "messages/payload_reading.hpp"
#include "messages/payload_reading.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Keep checks active in Release builds too.
#define CHECK(condition) do { \
    if (!(condition)) { \
        std::fprintf(stderr, "Check failed at line %d: %s\n", __LINE__, #condition); \
        std::exit(1); \
    } \
} while (false)

void golden_bytes_and_capacities() {
    PayloadReading<1> input;
    input.imu_present = true;
    input.imu.gyro[0] = {-2, -500000};
    input.imu.gyro[1] = {INT32_MIN, INT32_MAX};
    input.imu.accel[2] = {9, 500000};
    input.sweep_count = 1;
    input.sweeps[0].amu_id = 2;
    auto& sweep = input.sweeps[0].sweep;
    sweep.meta = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0x12345678, 0xaabbccdd};
    sweep.iv.voltage.fill(1.0F);
    sweep.iv.current.fill(-2.5F);

    // Independent golden vector: mask, signed IMU components, IEEE-754
    // metadata, timestamp/CRC, then voltage and current arrays.
    const std::uint8_t header_and_imu[] = {
        0x09, 0x00, 0x00, 0x00,
        0xfe, 0xff, 0xff, 0xff, 0xe0, 0x5e, 0xf8, 0xff,
        0x00, 0x00, 0x00, 0x80, 0xff, 0xff, 0xff, 0x7f,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x09, 0x00, 0x00, 0x00, 0x20, 0xa1, 0x07, 0x00,
    };
    const std::uint8_t metadata[] = {
        0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x40,
        0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x80, 0x40,
        0x00, 0x00, 0xa0, 0x40, 0x00, 0x00, 0xc0, 0x40,
        0x00, 0x00, 0xe0, 0x40, 0x00, 0x00, 0x00, 0x41,
        0x00, 0x00, 0x10, 0x41, 0x00, 0x00, 0x20, 0x41,
        0x78, 0x56, 0x34, 0x12, 0xdd, 0xcc, 0xbb, 0xaa,
    };
    std::array<std::uint8_t, 420> expected{};
    std::copy_n(header_and_imu, 52, expected.begin());
    std::copy_n(metadata, 48, expected.begin() + 52);
    for (std::size_t i = 0; i < 40; ++i) {
        expected[100 + i * 4 + 2] = 0x80;
        expected[100 + i * 4 + 3] = 0x3f;
        expected[260 + i * 4 + 2] = 0x20;
        expected[260 + i * 4 + 3] = 0xc0;
    }

    Packet<421> packet(MessageType::ack);
    packet.data.fill(0xaa);
    CHECK(encode(input, packet) == CodecStatus::ok);
    CHECK(packet.type == MessageType::payload_reading);
    CHECK(packet.size == expected.size());
    CHECK(std::equal(expected.begin(), expected.end(), packet.data.begin()));
    CHECK(packet.data[420] == 0xaa);  // No padding or capacity bytes written.

    PayloadReading<31> output;
    CHECK(decode(expected.data(), expected.size(), output) == CodecStatus::ok);
    CHECK(output.sweep_count == 1 && output.sweeps[0].amu_id == 2);
    CHECK(output.imu.gyro[0].whole == -2 && output.imu.gyro[0].micro == -500000);
    CHECK(output.imu.gyro[1].whole == INT32_MIN);
    CHECK(output.imu.gyro[1].micro == INT32_MAX);
    CHECK(output.imu.accel[2].micro == 500000);
    CHECK(output.sweeps[0].sweep.meta.adc == 10);
    CHECK(output.sweeps[0].sweep.meta.timestamp == 0x12345678);
    CHECK(output.sweeps[0].sweep.meta.crc == 0xaabbccdd);
    CHECK(output.sweeps[0].sweep.iv.voltage[39] == 1.0F);
    CHECK(output.sweeps[0].sweep.iv.current[39] == -2.5F);

    Packet<420> encoded_again(MessageType::payload_reading);
    CHECK(encode(output, encoded_again) == CodecStatus::ok);
    CHECK(encoded_again.data == expected);
    PayloadReading<1> small_output;
    CHECK(decode(encoded_again.view(), small_output) == CodecStatus::ok);

    // Every truncation, and trailing bytes, are rejected before changing output.
    for (std::size_t size = 0; size < expected.size(); ++size) {
        CHECK(decode(expected.data(), size, output) == CodecStatus::invalid_length);
    }
    CHECK(decode(packet.data.data(), packet.data.size(), output) == CodecStatus::invalid_length);
    CHECK(decode(nullptr, 420, output) == CodecStatus::invalid_argument);
    auto wrong_type = packet.view();
    wrong_type.type = MessageType::ack;
    CHECK(decode(wrong_type, output) == CodecStatus::wrong_message_type);
    wrong_type.type = static_cast<MessageType>(0xff);
    CHECK(decode(wrong_type, output) == CodecStatus::wrong_message_type);
    CHECK(encode(output, encoded_again) == CodecStatus::ok);
    CHECK(encoded_again.data == expected);  // Failed decodes preserved all active fields.
}

void sparse_ids_and_errors() {
    PayloadReading<2> input;
    input.sweep_count = 2;
    input.sweeps[0].amu_id = 30;  // Highest identity fits even with capacity 2.
    input.sweeps[0].sweep.meta.voc = 30;
    input.sweeps[1].amu_id = 2;
    input.sweeps[1].sweep.meta.voc = 2;
    Packet<740> packet(MessageType::payload_reading);
    CHECK(encode(input, packet) == CodecStatus::ok);
    CHECK(packet.size == 740);
    CHECK(packet.data[0] == 8 && packet.data[3] == 0x80);
    PayloadReading<2> output;
    CHECK(decode(packet.view(), output) == CodecStatus::ok);
    CHECK(output.sweeps[0].amu_id == 2 && output.sweeps[0].sweep.meta.voc == 2);
    CHECK(output.sweeps[1].amu_id == 30 && output.sweeps[1].sweep.meta.voc == 30);

    PayloadReading<1> too_small;
    too_small.imu_present = true;
    too_small.imu.accel[0].whole = 42;
    too_small.sweep_count = 1;
    too_small.sweeps[0].amu_id = 7;
    CHECK(decode(packet.view(), too_small) == CodecStatus::capacity_exceeded);
    CHECK(too_small.imu_present && too_small.imu.accel[0].whole == 42);
    CHECK(too_small.sweep_count == 1 && too_small.sweeps[0].amu_id == 7);

    const auto unchanged = packet.data;
    input.sweeps[1].amu_id = 30;
    CHECK(encode(input, packet) == CodecStatus::invalid_reading);
    input.sweeps[1].amu_id = 31;
    CHECK(encode(input, packet) == CodecStatus::invalid_reading);
    input.sweep_count = 3;
    CHECK(encode(input, packet) == CodecStatus::invalid_reading);
    CHECK(packet.data == unchanged && packet.size == 740);
    input.sweep_count = 2;
    input.sweeps[1].amu_id = 2;

    Packet<739> short_packet(MessageType::ack);
    short_packet.size = 1;
    short_packet.data.fill(0x55);
    CHECK(encode(input, short_packet) == CodecStatus::buffer_too_small);
    CHECK(short_packet.type == MessageType::ack && short_packet.size == 1);
    CHECK(std::all_of(short_packet.data.begin(), short_packet.data.end(),
                      [](auto byte) { return byte == 0x55; }));
    std::size_t written = 123;
    CHECK(encode(input, nullptr, 740, written) == CodecStatus::invalid_argument);
    CHECK(written == 0);
}

void empty_imu_and_maximum() {
    PayloadReading<0> empty;
    Packet<52> packet(MessageType::ack);
    CHECK(encode(empty, packet) == CodecStatus::ok);
    CHECK(packet.size == 4 && packet.data[0] == 0);
    PayloadReading<0> output;
    output.imu_present = true;
    CHECK(decode(packet.view(), output) == CodecStatus::ok);
    CHECK(!output.imu_present && output.sweep_count == 0);
    empty.imu_present = true;
    CHECK(encode(empty, packet) == CodecStatus::ok);
    CHECK(packet.size == 52 && packet.data[0] == 1);
    CHECK(decode(packet.view(), output) == CodecStatus::ok && output.imu_present);
    Packet<0> no_room(MessageType::ack);
    CHECK(encode(empty, no_room) == CodecStatus::buffer_too_small);

    PayloadReading<31> full;
    full.imu_present = true;
    full.sweep_count = 31;
    for (std::size_t i = 0; i < 31; ++i) {
        full.sweeps[i].amu_id = static_cast<std::uint8_t>(30 - i);
    }
    Packet<PayloadReading<31>::max_encoded_size> full_packet(MessageType::ack);
    CHECK(encode(full, full_packet) == CodecStatus::ok);
    CHECK(full_packet.size == full_packet.data.size());
    CHECK(std::all_of(full_packet.data.begin(), full_packet.data.begin() + 4,
                      [](auto byte) { return byte == 0xff; }));
    PayloadReading<31> decoded;
    CHECK(decode(full_packet.view(), decoded) == CodecStatus::ok);
    CHECK(decoded.sweep_count == 31 && decoded.sweeps[30].amu_id == 30);
}

void float_bits_are_preserved() {
    // Valid single-sweep body with representative special IEEE-754 values in
    // the first four metadata fields: -0, +infinity, -infinity, and quiet NaN.
    std::array<std::uint8_t, 372> bytes{};
    bytes[0] = 2;  // AMU 0 present, no IMU.
    const std::uint8_t special[] = {
        0x00, 0x00, 0x00, 0x80,
        0x00, 0x00, 0x80, 0x7f,
        0x00, 0x00, 0x80, 0xff,
        0x45, 0x23, 0xc1, 0x7f,
    };
    std::copy_n(special, sizeof(special), bytes.begin() + 4);
    PayloadReading<1> reading;
    CHECK(decode(bytes.data(), bytes.size(), reading) == CodecStatus::ok);
    Packet<372> packet(MessageType::payload_reading);
    CHECK(encode(reading, packet) == CodecStatus::ok);
    CHECK(packet.data == bytes);
}

int main() {
    golden_bytes_and_capacities();
    sparse_ids_and_errors();
    empty_imu_and_maximum();
    float_bits_are_preserved();
    std::puts("PayloadReading codec checks passed");
}
