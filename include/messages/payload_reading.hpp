#pragma once

#include "codec_status.hpp"
#include "packet.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

// Protocol constants, independent of driver settings and DeviceTree.
inline constexpr std::size_t kPayloadMaxAmus = 31;
inline constexpr std::size_t kPayloadIvPoints = 40;

// Preserves Zephyr sensor_value's two-part representation without depending on
// Zephyr: value = whole + micro / 1,000,000. No quantization or normalization.
struct SensorValue {
    std::int32_t whole = 0;
    std::int32_t micro = 0;
};

struct ImuReading {
    std::array<SensorValue, 3> gyro{};   // x, y, z in radians/second
    std::array<SensorValue, 3> accel{};  // x, y, z in meters/second squared
};

// The existing AMU metadata fields, retaining their driver-provided values.
// crc is source metadata, not an integrity check for this protocol packet.
struct SweepMetadata {
    float voc = 0;
    float isc = 0;
    float tsensor_start = 0;
    float tsensor_end = 0;
    float ff = 0;
    float eff = 0;
    float vmax = 0;
    float imax = 0;
    float pmax = 0;
    float adc = 0;
    std::uint32_t timestamp = 0;
    std::uint32_t crc = 0;
};

struct SweepPoints {
    std::array<float, kPayloadIvPoints> voltage{};
    std::array<float, kPayloadIvPoints> current{};
};

struct IvSweep {
    SweepMetadata meta{};
    SweepPoints iv{};
};

struct IndexedSweep {
    std::uint8_t amu_id = 0;  // Stable identity in 0..30, not an array index.
    IvSweep sweep{};
};

template<std::size_t Capacity>
struct PayloadReading {
    static_assert(Capacity <= kPayloadMaxAmus,
                  "Payload presence mask supports at most 31 AMUs");

    // Four mask bytes, six two-part IMU values, and up to Capacity sweeps.
    static constexpr std::size_t max_encoded_size =
        4 + 48 + Capacity * (48 + kPayloadIvPoints * 8);

    bool imu_present = false;
    ImuReading imu{};
    std::uint8_t sweep_count = 0;
    // Only [0, sweep_count) is meaningful. Entries may be supplied in any order.
    std::array<IndexedSweep, Capacity> sweeps{};
};

namespace payload_reading_detail {

static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559,
              "Payload readings require IEEE-754 binary32 floats");

inline constexpr std::size_t kMaskBytes = 4;
inline constexpr std::size_t kImuBytes = 48;
inline constexpr std::size_t kSweepBytes = 48 + kPayloadIvPoints * 8;

// These cursors operate only after the public codecs validate the full size.
struct Writer {
    std::uint8_t* cursor;

    void u32(std::uint32_t value) noexcept {
        for (unsigned shift = 0; shift < 32; shift += 8) {
            *cursor++ = static_cast<std::uint8_t>(value >> shift);
        }
    }

    void i32(std::int32_t value) noexcept {
        u32(static_cast<std::uint32_t>(value));
    }

    void f32(const float& value) noexcept {
        std::uint32_t bits;
        std::memcpy(&bits, &value, sizeof(bits));
        u32(bits);
    }
};

struct Reader {
    const std::uint8_t* cursor;

    std::uint32_t u32() noexcept {
        std::uint32_t value = 0;
        for (unsigned shift = 0; shift < 32; shift += 8) {
            value |= static_cast<std::uint32_t>(*cursor++) << shift;
        }
        return value;
    }

    std::int32_t i32() noexcept {
        const auto bits = u32();
        // Avoid implementation-defined unsigned-to-signed overflow in C++17.
        if (bits <= static_cast<std::uint32_t>(INT32_MAX)) {
            return static_cast<std::int32_t>(bits);
        }
        return -1 - static_cast<std::int32_t>(UINT32_MAX - bits);
    }

    void f32(float& value) noexcept {
        const auto bits = u32();
        std::memcpy(&value, &bits, sizeof(value));
    }
};

inline void write_imu(Writer& writer, const ImuReading& imu) noexcept {
    for (const auto& axis : imu.gyro) {
        writer.i32(axis.whole);
        writer.i32(axis.micro);
    }
    for (const auto& axis : imu.accel) {
        writer.i32(axis.whole);
        writer.i32(axis.micro);
    }
}

inline void read_imu(Reader& reader, ImuReading& imu) noexcept {
    for (auto& axis : imu.gyro) {
        axis.whole = reader.i32();
        axis.micro = reader.i32();
    }
    for (auto& axis : imu.accel) {
        axis.whole = reader.i32();
        axis.micro = reader.i32();
    }
}

inline void write_sweep(Writer& writer, const IvSweep& sweep) noexcept {
    writer.f32(sweep.meta.voc);
    writer.f32(sweep.meta.isc);
    writer.f32(sweep.meta.tsensor_start);
    writer.f32(sweep.meta.tsensor_end);
    writer.f32(sweep.meta.ff);
    writer.f32(sweep.meta.eff);
    writer.f32(sweep.meta.vmax);
    writer.f32(sweep.meta.imax);
    writer.f32(sweep.meta.pmax);
    writer.f32(sweep.meta.adc);
    writer.u32(sweep.meta.timestamp);
    writer.u32(sweep.meta.crc);
    for (const auto& voltage : sweep.iv.voltage) {
        writer.f32(voltage);
    }
    for (const auto& current : sweep.iv.current) {
        writer.f32(current);
    }
}

inline void read_sweep(Reader& reader, IvSweep& sweep) noexcept {
    reader.f32(sweep.meta.voc);
    reader.f32(sweep.meta.isc);
    reader.f32(sweep.meta.tsensor_start);
    reader.f32(sweep.meta.tsensor_end);
    reader.f32(sweep.meta.ff);
    reader.f32(sweep.meta.eff);
    reader.f32(sweep.meta.vmax);
    reader.f32(sweep.meta.imax);
    reader.f32(sweep.meta.pmax);
    reader.f32(sweep.meta.adc);
    sweep.meta.timestamp = reader.u32();
    sweep.meta.crc = reader.u32();
    for (auto& voltage : sweep.iv.voltage) {
        reader.f32(voltage);
    }
    for (auto& current : sweep.iv.current) {
        reader.f32(current);
    }
}

} // namespace payload_reading_detail

// Body layout: little-endian u32 presence mask, optional IMU, then sweeps in
// ascending stable-ID order. Bit 0 selects IMU; bit (1 + amu_id) selects a sweep.
// Capacity is never encoded. Source and destination storage must not overlap.
// On failure, written is zero and destination bytes are unchanged.
template<std::size_t Capacity>
CodecStatus encode(const PayloadReading<Capacity>& reading,
                   std::uint8_t* destination, std::size_t destination_capacity,
                   std::size_t& written) noexcept {
    using namespace payload_reading_detail;
    written = 0;
    if (reading.sweep_count > Capacity) {
        return CodecStatus::invalid_reading;
    }

    std::uint32_t mask = reading.imu_present ? 1U : 0U;
    for (std::size_t i = 0; i < reading.sweep_count; ++i) {
        const auto id = reading.sweeps[i].amu_id;
        if (id >= kPayloadMaxAmus) {
            return CodecStatus::invalid_reading;
        }
        const auto bit = std::uint32_t{1} << (id + 1);
        if ((mask & bit) != 0) {
            return CodecStatus::invalid_reading;
        }
        mask |= bit;
    }

    const auto size = kMaskBytes + (reading.imu_present ? kImuBytes : 0) +
                      reading.sweep_count * kSweepBytes;
    if (destination_capacity < size) {
        return CodecStatus::buffer_too_small;
    }
    if (destination == nullptr) {
        return CodecStatus::invalid_argument;
    }

    Writer writer{destination};
    writer.u32(mask);
    if (reading.imu_present) {
        write_imu(writer, reading.imu);
    }
    // Sort on the wire without allocating or copying a temporary sweep array.
    for (std::size_t id = 0; id < kPayloadMaxAmus; ++id) {
        for (std::size_t i = 0; i < reading.sweep_count; ++i) {
            if (reading.sweeps[i].amu_id == id) {
                write_sweep(writer, reading.sweeps[i].sweep);
                break;
            }
        }
    }
    written = size;
    return CodecStatus::ok;
}

// All structural checks happen before changing destination. Unused array slots
// (and imu when absent) are left untouched and must not be interpreted as data.
// Source and destination storage must not overlap.
template<std::size_t Capacity>
CodecStatus decode(const std::uint8_t* source, std::size_t size,
                   PayloadReading<Capacity>& destination) noexcept {
    using namespace payload_reading_detail;
    if (source == nullptr) {
        return CodecStatus::invalid_argument;
    }
    if (size < kMaskBytes) {
        return CodecStatus::invalid_length;
    }

    Reader reader{source};
    const auto mask = reader.u32();
    const bool has_imu = (mask & 1U) != 0;
    std::size_t count = 0;
    for (std::size_t id = 0; id < kPayloadMaxAmus; ++id) {
        if ((mask & (std::uint32_t{1} << (id + 1))) != 0) {
            ++count;
        }
    }
    const auto expected = kMaskBytes + (has_imu ? kImuBytes : 0) + count * kSweepBytes;
    if (size != expected) {
        return CodecStatus::invalid_length;
    }
    if (count > Capacity) {
        return CodecStatus::capacity_exceeded;
    }

    destination.imu_present = has_imu;
    destination.sweep_count = static_cast<std::uint8_t>(count);
    if (has_imu) {
        read_imu(reader, destination.imu);
    }
    std::size_t index = 0;
    for (std::size_t id = 0; id < kPayloadMaxAmus; ++id) {
        if ((mask & (std::uint32_t{1} << (id + 1))) != 0) {
            auto& entry = destination.sweeps[index++];
            entry.amu_id = static_cast<std::uint8_t>(id);
            read_sweep(reader, entry.sweep);
        }
    }
    return CodecStatus::ok;
}

// Packet helpers assign/check the message type. The packet body does not
// include a wire header. A failed encode leaves the entire packet unchanged.
template<std::size_t ReadingCapacity, std::size_t PacketCapacity>
CodecStatus encode(const PayloadReading<ReadingCapacity>& reading,
                   Packet<PacketCapacity>& packet) noexcept {
    std::size_t written = 0;
    const auto status = encode(reading, packet.data.data(), PacketCapacity, written);
    if (status == CodecStatus::ok) {
        packet.type = MessageType::payload_reading;
        packet.size = static_cast<std::uint16_t>(written);
    }
    return status;
}

template<std::size_t Capacity>
CodecStatus decode(PacketView packet, PayloadReading<Capacity>& reading) noexcept {
    if (packet.type != MessageType::payload_reading) {
        return CodecStatus::wrong_message_type;
    }
    return decode(packet.data, packet.size, reading);
}
