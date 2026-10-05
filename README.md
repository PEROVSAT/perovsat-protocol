# PEROVSAT protocol

A header-only C++17 library shared by flight and ground software. The initial
PayloadReading port has no Zephyr, DeviceTree, driver, filesystem, heap,
exception, or RTTI dependency.

## Start reviewing here

- `include/message_type.hpp`: stable message IDs.
- `include/packet.hpp`: encoded-byte views and fixed-capacity packet storage.
- `include/messages/payload_reading.hpp`: portable measurement structs and codecs.
- `examples/payload_reading.cpp`: one enabled AMU, ID 2, encoded on a bench build
  and decoded with the same library into a larger ground-side object.
- `tests/payload_reading_test.cpp`: golden bytes, capacity interoperability,
  canonical ordering, and invalid-input behavior.

`messages` names the data types; each type's **format** is its byte layout and its
**codec** is the implementation of encode/decode.

## Capacity and identity

`PayloadReading<N>` reserves room for N AMU sweeps, with an optional IMU reading.
N may be 0 through 31. Only the first `sweep_count` entries are active. Each entry
has a stable `amu_id` in 0..30, regardless of N. Entries need not be sorted.

For example, `PayloadReading<1>` can hold AMU 30. A later flight adapter can use
`PayloadReading<NUM_AMUS>` with a DT-derived enabled-device count. It must map
each enabled device to its stable ID rather than renumbering IDs when other
devices are disabled. That adapter is not part of this port.

Capacity is a local RAM limit and never appears on the wire. A decoder accepts
any reading whose actual sweep count fits its destination, even when the sender
has a different capacity. A valid larger reading returns `capacity_exceeded`;
it is not silently truncated.

The structs are plain data. Templated free functions implement encoding and
decoding; a class with member functions would not change capacity handling.

## API

```cpp
#include "messages/payload_reading.hpp"

PayloadReading<1> reading;
reading.sweep_count = 1;
reading.sweeps[0].amu_id = 2;
reading.sweeps[0].sweep.meta.voc = 1.25F;

Packet<PayloadReading<1>::max_encoded_size> packet(MessageType::payload_reading);
const auto encoded = encode(reading, packet);
if (encoded == CodecStatus::ok) {
    PayloadReading<31> decoded;
    const auto status = decode(packet.view(), decoded);
    // Use decoded only when status == CodecStatus::ok.
}
```

The packet overload sets `type` and the actual body `size` on success. Packet
capacity only needs to fit the currently included measurements, not the source
object's maximum capacity. `max_encoded_size` includes optional IMU data.

Raw buffer overloads are also available:

```cpp
CodecStatus encode(const PayloadReading<N>& reading,
                   std::uint8_t* destination, std::size_t capacity,
                   std::size_t& written);
CodecStatus decode(const std::uint8_t* source, std::size_t size,
                   PayloadReading<N>& destination);
```

The caller owns all storage and provides its true available size. Input and
output storage must not overlap. Encoding validates the count, ID range,
uniqueness, and buffer capacity before writing anything. Decoding validates the
entire body length and destination capacity before changing any fields. Failed
operations preserve output data; raw encoding sets `written` to zero on failure.
The packet decoder also verifies the message type.

Successful decoding sets presence/count and writes active data in ascending ID
order. Inactive array slots and an absent IMU are left untouched. Interpret
them only through `sweep_count` and `imu_present`; this avoids clearing or
copying a maximum-size temporary object. Packet views borrow storage, which must
remain alive and unchanged until consumers have finished using it.

## Draft PayloadReading body layout

All multi-byte fields use little-endian byte order and are written individually.
There is no native struct padding, pointer, capacity, or separate ID list in the
encoded body.

1. A four-byte unsigned presence mask: bit 0 means IMU present; bit `1 + amu_id`
   means that AMU's sweep is present.
2. If present, IMU gyro x/y/z, then acceleration x/y/z. Each axis is two signed
   32-bit two's-complement values: `whole`, then `micro`, representing
   `whole + micro / 1,000,000`. Gyro uses rad/s; acceleration uses m/s². These
   components preserve the existing Zephyr sensor values without normalization.
3. For each present AMU, in ascending ID order:
   - Ten IEEE-754 binary32 values: `voc`, `isc`, `tsensor_start`, `tsensor_end`,
     `ff`, `eff`, `vmax`, `imax`, `pmax`, `adc`.
   - Unsigned 32-bit `timestamp`, then unsigned 32-bit `crc`.
   - Forty binary32 voltage values, then forty binary32 current values.

The body size is `4 + (imu_present ? 48 : 0) + sweep_count * 368` bytes.
One sweep without an IMU is 372 bytes, independent of storage capacity. An empty
reading is a four-byte zero mask. All 31 sweeps plus an IMU take 11,460 bytes.

The codec preserves source measurement bits, including non-finite float values;
it validates structure, not scientific plausibility. AMU metadata retains the
driver's existing meanings. In particular, its timestamp epoch and metadata
units should be documented from the AMU specification before the format is
frozen. The AMU `crc` field is carried as metadata, not computed or verified as a
packet checksum.

This initial port retains the existing mask and measurement order but specifies
the bytes explicitly. It is not a general legacy `.raw` importer: old files
depended on native layout, driver configuration, and build-dependent IDs. It
does not add further compression, record identity, or a schema-version field.
Forty points is a protocol constant, not an `IV_POINTS` build override.

The packet's serialized envelope, versioning policy, integrity checks, and
transport fragmentation are still to be defined. `Packet::view()` describes the
body and type in memory; it does not serialize an envelope. The radio frame
limit is separate from this message's size. Absent sweeps mean only "no reading
included"; they do not distinguish disabled hardware from sampling failures.

## Build and check on a host

```sh
cmake -S . -B build -DPEROVSAT_PROTOCOL_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Consumers can add this repository as a CMake subdirectory and link the
`perovsat_protocol` interface target, or include the headers directly using
C++17 or newer. Tests and the example are opt-in, so including the library does
not add host executables to a flight build.
