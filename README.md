# PerovSat protocol

This repository defines the portable byte codec and flight measurement value
types shared by PerovSat flight and ground software. The public API is C11, with
headers that can also be included from C++. It has no heap, RTOS, hardware,
filesystem, or driver dependency.

## Layout

- `core/` contains the bounded reader and writer plus shared status codes.
- `flight/imu/`, `flight/sweep/`, and `flight/payload_reading/` contain the
  corresponding value types and codecs.
- `examples/` contains small C and C++ consumer examples.
- `simulation/` is reserved for future simulation support; no simulation codec
  is implemented in this pass.

The CMake targets are `perovsat_protocol_core`, `perovsat_protocol_flight`,
and the aggregate `perovsat_protocol` interface target. The root include path
lets consumers write, for example, `#include "core/writer.h"` or
`#include "flight/payload_reading/payload_reading.h"`.

The former header-only C++ prototype API has been replaced by compiled C
libraries. Consumers should link `perovsat_protocol_core` for the codec core,
or `perovsat_protocol_flight` / `perovsat_protocol` for the flight codecs.

## API and codec contract

Each value type uses the same cursor-first form:

```c
psp_payload_reading_serialize(&writer, &reading);
psp_payload_reading_deserialize(&reader, &reading);
```

Initialize a writer or reader with a caller-owned byte buffer first. Serialization
returns a `psp_status`; on success, `psp_writer_position(&writer)` is the actual
encoded length. The `*_MAX_SERIALIZED_SIZE` constants describe maximum required
capacity, not the length of every value. Capacity-based payload sizing is
available through `PSP_PAYLOAD_READING_MAX_SERIALIZED_SIZE_FOR(capacity)`.

Reader and writer cursor state, and the coupled optional/count state of a
payload reading, live in an `internal` member. Simple measurement fields remain
public. Use the payload init, clear, set, and getter functions to manage its
coupled state. Reader and writer errors are sticky, preserving the first failure.
Serialization or deserialization may have partially changed the output buffer
or value when an error occurs, so use output only after success. Input and output
byte buffers must not overlap. Nested codecs consume only their fields; call
`psp_reader_finish` at the outer body boundary to reject trailing bytes.

Payload storage is borrowed. A copied `psp_payload_reading` view still points to
the same caller-owned sweep array, which must outlive every copy. Getter pointers
refer into that array and should be treated as invalid after any mutation of the
reading. The declarations document the detailed contracts.

## PayloadReading body

All multi-byte fields are encoded explicitly in little-endian order, with no
native struct padding, pointers, storage capacity, envelope, or version field.
The body is:

1. A four-byte unsigned presence mask. Bit 0 indicates IMU data; bit `1 + id`
   indicates the sweep for stable AMU ID 0 through 30.
2. Optional IMU data: gyro x/y/z then acceleration x/y/z. Each axis is a signed
   32-bit `whole` followed by signed 32-bit `micro`, for 48 bytes total.
3. Included sweeps sorted by stable AMU ID. Each has ten binary32 metadata
   values (`voc`, `isc`, `tsensor_start`, `tsensor_end`, `ff`, `eff`, `vmax`,
   `imax`, `pmax`, `adc`), two unsigned 32-bit values (`timestamp`, `crc`),
   forty binary32 voltages, then forty binary32 currents: 368 bytes.

The body size is `4 + (imu_present ? 48 : 0) + sweep_count * 368`. An empty
reading is 4 bytes; a maximum reading with IMU and 31 sweeps is 11,460 bytes.
The number of bytes depends on actual content, never on local storage capacity.
A capacity-1 sender and capacity-31 receiver use the same format. A decoder
rejects a valid body whose sweep count exceeds its supplied storage capacity.

PayloadReading borrows caller-owned sweep storage. Keep that storage alive for
the full lifetime of the reading view. IDs remain stable when some AMUs are
disabled; use the payload index as the ID rather than compacting enabled
devices. Float values are preserved as encoded; the codec validates structure,
not scientific plausibility. The AMU CRC is carried as metadata and is not
computed or checked as a packet checksum.

## Host build

The default build compiles only the C11 libraries and requires no C++ compiler.
Enable host tests and examples with:

```sh
cmake -S . -B build -DPEROVSAT_PROTOCOL_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

The C example shows initialization, mutation, serialization, and exact input
consumption. The C++ example demonstrates that the public C headers and C
linkage compose from C++ code.

## Flight software integration

This repository defines protocol types; it does not migrate the application or
Zephyr driver. The application remains responsible for its
`payload::PayloadReading` save/load wrapper and driver-facing structs. An
adapter can map `amu_sweep_meta_t` fields by name, copy the 40 voltage/current
points, and map each sensor value's `val1` and `val2` to `whole` and `micro`.
Do not cast packed driver metadata to protocol structs. Size borrowed storage
for the configured `NUM_AMUS`, and assign each active sweep its stable payload
index. This keeps the codec independent of Zephyr and AMU implementation
details.

This pass implements the PayloadReading body codec only. It does not define a
Message or WirePacket abstraction, an envelope, packet versioning, transport
fragmentation, or simulation behavior.
