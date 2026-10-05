#pragma once

enum class CodecStatus {
    ok,
    invalid_argument,
    invalid_reading,
    buffer_too_small,
    invalid_length,
    capacity_exceeded,
    wrong_message_type,
};
