#pragma once

#include <cstdint>

// These IDs are part of the protocol: never renumber or reuse assigned values.
// Decoders must reject or explicitly handle unknown IDs.
enum class MessageType : std::uint8_t {
    ack = 0x00,

    payload_reading = 0x10
};
