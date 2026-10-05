#pragma once

#include "message_type.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

// A non-owning view of an encoded message body, excluding any wire header.
// data must address at least size readable bytes; nullptr is valid only for an
// empty body. The backing storage must remain alive and unchanged during use.
// This struct's native memory layout is not a serialized packet format.
struct PacketView {
    MessageType type;
    const std::uint8_t* data;
    std::uint16_t size;
};

// Fixed-capacity storage with no dynamic allocation. Capacity is a body-size
// bound, independent of a transport's frame limit. Only the first size bytes
// are meaningful; encoders must enforce size <= Capacity before setting size.
// Serialize the header and used body bytes explicitly, never this object.
template<std::size_t Capacity>
struct Packet {
    static_assert(Capacity <= std::numeric_limits<std::uint16_t>::max(),
                  "Packet capacity must fit in the protocol body length");

    MessageType type;
    std::uint16_t size = 0;
    std::array<std::uint8_t, Capacity> data{};

    explicit constexpr Packet(MessageType message_type) noexcept
        : type(message_type) {}

    constexpr PacketView view() const noexcept {
        return {type, data.data(), size};
    }
};
