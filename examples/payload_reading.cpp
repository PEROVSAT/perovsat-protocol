#include "messages/payload_reading.hpp"

int main() {
    // A bench build enables one AMU, whose stable identity is still 2.
    // Flight software could supply its DT-derived NUM_AMUS instead of 1.
    PayloadReading<1> bench;
    bench.sweep_count = 1;
    bench.sweeps[0].amu_id = 2;
    bench.sweeps[0].sweep.meta.voc = 1.25F;
    bench.sweeps[0].sweep.iv.voltage[0] = 0.5F;

    Packet<PayloadReading<1>::max_encoded_size> packet(MessageType::payload_reading);
    if (encode(bench, packet) != CodecStatus::ok) {
        return 1;
    }
    // 4-byte mask + one 368-byte sweep; absent IMU consumes no bytes.
    if (packet.size != 372) {
        return 2;
    }

    // Ground software uses the exact same decoder with room for all AMUs.
    PayloadReading<kPayloadMaxAmus> ground;
    if (decode(packet.view(), ground) != CodecStatus::ok) {
        return 3;
    }
    if (ground.sweep_count != 1 || ground.sweeps[0].amu_id != 2 ||
        ground.sweeps[0].sweep.meta.voc != 1.25F) {
        return 4;
    }

    // A larger in-memory capacity does not require a larger encoded packet.
    // This succeeds because ground currently contains only one sweep.
    if (encode(ground, packet) != CodecStatus::ok) {
        return 5;
    }
    PayloadReading<1> back_on_bench;
    return decode(packet.view(), back_on_bench) == CodecStatus::ok ? 0 : 6;
}
