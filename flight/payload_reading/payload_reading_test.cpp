#include "payload_reading.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>

#define CHECK(condition) do { if (!(condition)) { \
    std::fprintf(stderr, "failed line %d: %s\n", __LINE__, #condition); std::exit(1); \
} } while (false)

static void test_golden_vector_and_truncation() {
    psp_indexed_sweep source_storage[1]{};
    psp_payload_reading source{};
    CHECK(psp_payload_reading_init(&source, source_storage, 1) == PSP_OK);
    psp_imu_reading imu{};
    imu.gyro[0] = {-2, -500000};
    imu.gyro[1] = {INT32_MIN, INT32_MAX};
    imu.accel[2] = {9, 500000};
    CHECK(psp_payload_reading_set_imu(&source, &imu) == PSP_OK);

    psp_sweep sweep{};
    sweep.meta = {1,2,3,4,5,6,7,8,9,10,0x12345678u,0xaabbccddu};
    std::fill(std::begin(sweep.iv.voltage), std::end(sweep.iv.voltage), 1.0f);
    std::fill(std::begin(sweep.iv.current), std::end(sweep.iv.current), -2.5f);
    CHECK(psp_payload_reading_set_sweep(&source, 2, &sweep) == PSP_OK);

    const uint8_t header_imu[52] = {
        0x09,0,0,0, 0xfe,0xff,0xff,0xff, 0xe0,0x5e,0xf8,0xff,
        0,0,0,0x80, 0xff,0xff,0xff,0x7f,
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        0,0,0,0, 0,0,0,0, 9,0,0,0, 0x20,0xa1,7,0
    };
    const uint8_t metadata[48] = {
        0x00,0x00,0x80,0x3f, 0x00,0x00,0x00,0x40,
        0x00,0x00,0x40,0x40, 0x00,0x00,0x80,0x40,
        0x00,0x00,0xa0,0x40, 0x00,0x00,0xc0,0x40,
        0x00,0x00,0xe0,0x40, 0x00,0x00,0x00,0x41,
        0x00,0x00,0x10,0x41, 0x00,0x00,0x20,0x41,
        0x78,0x56,0x34,0x12, 0xdd,0xcc,0xbb,0xaa
    };
    std::array<uint8_t, 420> expected{};
    std::copy(std::begin(header_imu), std::end(header_imu), expected.begin());
    std::copy(std::begin(metadata), std::end(metadata), expected.begin()+52);
    for (size_t i=0;i<40;i++) {
        expected[100+i*4+2]=0x80; expected[100+i*4+3]=0x3f;
        expected[260+i*4+2]=0x20; expected[260+i*4+3]=0xc0;
    }
    std::array<uint8_t, 422> guarded{};
    guarded.fill(0xa5);
    psp_writer writer;
    psp_writer_init(&writer, guarded.data()+1, 420);
    CHECK(psp_payload_reading_serialize(&writer, &source)==PSP_OK);
    CHECK(psp_writer_position(&writer)==expected.size());
    CHECK(std::equal(expected.begin(),expected.end(),guarded.begin()+1));
    CHECK(guarded.front()==0xa5 && guarded.back()==0xa5);

    psp_indexed_sweep dest_storage[31]{};
    psp_payload_reading dest{};
    CHECK(psp_payload_reading_init(&dest,dest_storage,31)==PSP_OK);
    for(size_t n=0;n<expected.size();n++) {
        psp_reader reader;
        psp_reader_init(&reader,expected.data(),n);
        CHECK(psp_payload_reading_deserialize(&reader,&dest)==PSP_TRUNCATED_INPUT);
    }
    psp_reader reader;
    psp_reader_init(&reader,expected.data(),expected.size());
    CHECK(psp_payload_reading_deserialize(&reader,&dest)==PSP_OK);
    CHECK(psp_reader_finish(&reader)==PSP_OK);
    std::array<uint8_t,420> roundtrip{};
    psp_writer out;
    psp_writer_init(&out,roundtrip.data(),roundtrip.size());
    CHECK(psp_payload_reading_serialize(&out,&dest)==PSP_OK);
    CHECK(roundtrip==expected);
    CHECK(dest_storage[0].sweep.meta.timestamp==0x12345678u);
    CHECK(dest_storage[0].sweep.meta.crc==0xaabbccddu);
}

static void test_float_bits_and_composition() {
    std::array<uint8_t, 372> bytes{};
    bytes[0]=2; // AMU 0, no IMU.
    const uint8_t specials[] = {0,0,0,0x80, 0,0,0x80,0x7f, 0,0,0x80,0xff, 0x45,0x23,0xc1,0x7f};
    std::copy(std::begin(specials),std::end(specials),bytes.begin()+4);
    std::array<uint8_t,374> wrapped{};
    wrapped[0]=0xee; wrapped[373]=0xdd;
    std::copy(bytes.begin(),bytes.end(),wrapped.begin()+1);
    psp_reader reader;
    psp_reader_init(&reader,wrapped.data(),wrapped.size());
    psp_reader_skip(&reader,1);
    psp_indexed_sweep storage[1]{};
    psp_payload_reading value{};
    CHECK(psp_payload_reading_init(&value,storage,1)==PSP_OK);
    CHECK(psp_payload_reading_deserialize(&reader,&value)==PSP_OK);
    CHECK(psp_reader_position(&reader)==bytes.size()+1);
    uint8_t suffix=0;
    psp_read_u8(&reader,&suffix);
    CHECK(suffix==0xdd && psp_reader_finish(&reader)==PSP_OK);
    psp_reader trailing_reader;
    psp_reader_init(&trailing_reader,wrapped.data()+1,bytes.size()+1);
    CHECK(psp_payload_reading_deserialize(&trailing_reader,&value)==PSP_OK);
    CHECK(psp_reader_finish(&trailing_reader)==PSP_TRAILING_DATA);
    CHECK(wrapped.front()==0xee && wrapped.back()==0xdd);
    std::array<uint8_t,374> composed{};
    psp_writer composed_writer;
    psp_writer_init(&composed_writer,composed.data(),composed.size());
    psp_write_u8(&composed_writer,0xee);
    CHECK(psp_payload_reading_serialize(&composed_writer,&value)==PSP_OK);
    psp_write_u8(&composed_writer,0xdd);
    CHECK(psp_writer_position(&composed_writer)==composed.size());
    CHECK(composed.front()==0xee && composed.back()==0xdd);
    std::array<uint8_t,372> encoded{};
    psp_writer writer;
    psp_writer_init(&writer,encoded.data(),encoded.size());
    CHECK(psp_payload_reading_serialize(&writer,&value)==PSP_OK);
    CHECK(encoded==bytes);
}

static void test_capacity_ids_storage_and_mutators() {
    psp_indexed_sweep source_slots[2]{};
    psp_payload_reading source{};
    CHECK(psp_payload_reading_init(&source,source_slots,2)==PSP_OK);
    psp_sweep sw{}; sw.meta.voc=30;
    CHECK(psp_payload_reading_set_sweep(&source,30,&sw)==PSP_OK);
    sw.meta.voc=2;
    CHECK(psp_payload_reading_set_sweep(&source,2,&sw)==PSP_OK);
    uint8_t bytes[PSP_PAYLOAD_READING_MAX_SERIALIZED_SIZE_FOR(2)]{};
    psp_writer writer; psp_writer_init(&writer,bytes,sizeof(bytes));
    CHECK(psp_payload_reading_serialize(&writer,&source)==PSP_OK);
    CHECK(psp_writer_position(&writer)==740 && bytes[0]==8 && bytes[3]==0x80);
    psp_indexed_sweep thirty_one_slots[31]{}; psp_payload_reading wide{};
    CHECK(psp_payload_reading_init(&wide,thirty_one_slots,31)==PSP_OK);
    psp_reader wide_reader; psp_reader_init(&wide_reader,bytes,740);
    CHECK(psp_payload_reading_deserialize(&wide_reader,&wide)==PSP_OK);
    uint8_t back[740]{}; psp_writer back_writer; psp_writer_init(&back_writer,back,sizeof(back));
    CHECK(psp_payload_reading_serialize(&back_writer,&wide)==PSP_OK);
    CHECK(std::equal(std::begin(bytes),std::begin(bytes)+740,std::begin(back)));

    psp_indexed_sweep cap_one_storage[1]{};
    psp_payload_reading cap_one{};
    CHECK(psp_payload_reading_init(&cap_one,cap_one_storage,1)==PSP_OK);
    CHECK(psp_payload_reading_set_sweep(&cap_one,30,&sw)==PSP_OK);
    uint8_t one_wire[372]{}; psp_writer one_writer; psp_writer_init(&one_writer,one_wire,sizeof(one_wire));
    CHECK(psp_payload_reading_serialize(&one_writer,&cap_one)==PSP_OK);
    psp_reader one_reader; psp_reader_init(&one_reader,one_wire,sizeof(one_wire));
    CHECK(psp_payload_reading_deserialize(&one_reader,&wide)==PSP_OK);
    uint8_t one_back[372]{}; psp_writer one_back_writer; psp_writer_init(&one_back_writer,one_back,sizeof(one_back));
    CHECK(psp_payload_reading_serialize(&one_back_writer,&wide)==PSP_OK);
    psp_payload_reading cap_one_again{};
    CHECK(psp_payload_reading_init(&cap_one_again,cap_one_storage,1)==PSP_OK);
    psp_reader one_back_reader; psp_reader_init(&one_back_reader,one_back,sizeof(one_back));
    CHECK(psp_payload_reading_deserialize(&one_back_reader,&cap_one_again)==PSP_OK);
    CHECK(psp_payload_reading_get_sweep(&cap_one_again,30)!=nullptr);

    psp_indexed_sweep one_slot[1]{}; psp_payload_reading small{};
    CHECK(psp_payload_reading_init(&small,one_slot,1)==PSP_OK);
    psp_reader reader; psp_reader_init(&reader,bytes,740);
    CHECK(psp_payload_reading_deserialize(&reader,&small)==PSP_CAPACITY_EXCEEDED);
    CHECK(psp_payload_reading_set_sweep(&small,7,&sw)==PSP_OK);
    psp_sweep replacement{}; replacement.meta.voc=77;
    CHECK(psp_payload_reading_set_sweep(&small,7,&replacement)==PSP_OK);
    CHECK(psp_payload_reading_get_sweep(&small,7)->meta.voc==77);
    CHECK(psp_payload_reading_set_sweep(&small,8,&sw)==PSP_CAPACITY_EXCEEDED);
    CHECK(psp_payload_reading_sweep_count(&small)==1);
    CHECK(psp_payload_reading_get_sweep(&small,7)->meta.voc==77);
    CHECK(psp_payload_reading_get_sweep(&small,3)==nullptr);
    CHECK(psp_payload_reading_clear_sweep(&small,3)==PSP_OK);
    CHECK(psp_payload_reading_clear_sweep(&small,7)==PSP_OK);
    CHECK(psp_payload_reading_set_sweep(&small,8,&sw)==PSP_OK);
    CHECK(psp_payload_reading_clear(&small)==PSP_OK);
    CHECK(psp_payload_reading_sweep_count(&small)==0 && one_slot[0].amu_id==8);
    CHECK(psp_payload_reading_clear_imu(nullptr)==PSP_INVALID_ARGUMENT);
    CHECK(psp_payload_reading_clear(nullptr)==PSP_INVALID_ARGUMENT);

    psp_payload_reading zero{};
    CHECK(psp_payload_reading_init(&zero,nullptr,0)==PSP_OK);
    psp_imu_reading imu{}; imu.accel[0].whole=42;
    CHECK(psp_payload_reading_set_imu(&zero,&imu)==PSP_OK);
    CHECK(psp_payload_reading_get_imu(&zero)->accel[0].whole==42);
    CHECK(psp_payload_reading_clear_imu(&zero)==PSP_OK);
    CHECK(psp_payload_reading_get_imu(&zero)==nullptr);
    uint8_t empty_imu[52]{}; psp_writer_init(&writer,empty_imu,sizeof(empty_imu));
    CHECK(psp_payload_reading_set_imu(&zero,&imu)==PSP_OK);
    CHECK(psp_payload_reading_serialize(&writer,&zero)==PSP_OK);
    CHECK(empty_imu[0]==1 && psp_writer_position(&writer)==52);
    CHECK(psp_payload_reading_clear(&zero)==PSP_OK);
    uint8_t empty_wire[4]={0xff,0xff,0xff,0xff};
    psp_writer empty_writer; psp_writer_init(&empty_writer,empty_wire,sizeof(empty_wire));
    CHECK(psp_payload_reading_serialize(&empty_writer,&zero)==PSP_OK);
    CHECK(psp_writer_position(&empty_writer)==4 && empty_wire[0]==0 && empty_wire[1]==0 && empty_wire[2]==0 && empty_wire[3]==0);
    psp_payload_reading empty_out{};
    CHECK(psp_payload_reading_init(&empty_out,nullptr,0)==PSP_OK);
    psp_reader empty_reader; psp_reader_init(&empty_reader,empty_wire,sizeof(empty_wire));
    CHECK(psp_payload_reading_deserialize(&empty_reader,&empty_out)==PSP_OK);
    CHECK(psp_reader_finish(&empty_reader)==PSP_OK && psp_payload_reading_sweep_count(&empty_out)==0);

    psp_indexed_sweep full_slots[31]{}; psp_payload_reading full{};
    CHECK(psp_payload_reading_init(&full,full_slots,31)==PSP_OK);
    for(uint8_t id=0;id<31;id++) CHECK(psp_payload_reading_set_sweep(&full,id,&sw)==PSP_OK);
    CHECK(psp_payload_reading_sweep_count(&full)==31);
    CHECK(psp_payload_reading_set_imu(&full,&imu)==PSP_OK);
    std::array<uint8_t,PSP_PAYLOAD_READING_MAX_SERIALIZED_SIZE> maximum{};
    psp_writer_init(&writer,maximum.data(),maximum.size());
    CHECK(psp_payload_reading_serialize(&writer,&full)==PSP_OK);
    CHECK(psp_writer_position(&writer)==maximum.size() && maximum.size()==11460);
    psp_indexed_sweep decoded_slots[31]{}; psp_payload_reading decoded_full{};
    CHECK(psp_payload_reading_init(&decoded_full,decoded_slots,31)==PSP_OK);
    psp_reader full_reader; psp_reader_init(&full_reader,maximum.data(),maximum.size());
    CHECK(psp_payload_reading_deserialize(&full_reader,&decoded_full)==PSP_OK);
    CHECK(psp_reader_finish(&full_reader)==PSP_OK);
    CHECK(psp_payload_reading_sweep_count(&decoded_full)==31);
    CHECK(psp_payload_reading_get_sweep(&decoded_full,30)!=nullptr);
    CHECK(psp_payload_reading_get_imu(&decoded_full)!=nullptr);
}

static void test_short_writes_invalid_internal_state_and_sticky_errors() {
    psp_indexed_sweep slots[2]{}; psp_payload_reading value{};
    CHECK(psp_payload_reading_init(&value,slots,2)==PSP_OK);
    psp_sweep sw{};
    CHECK(psp_payload_reading_set_sweep(&value,2,&sw)==PSP_OK);
    for(size_t capacity=0;capacity<372;capacity++) {
        std::array<uint8_t,374> guarded{}; guarded.fill(0x5a);
        psp_writer writer; psp_writer_init(&writer,guarded.data()+1,capacity);
        CHECK(psp_payload_reading_serialize(&writer,&value)==PSP_BUFFER_TOO_SMALL);
        CHECK(guarded.front()==0x5a && guarded.back()==0x5a);
        CHECK(std::all_of(guarded.begin()+1+capacity,guarded.end(),
                          [](uint8_t byte){return byte==0x5a;}));
    }
    psp_writer writer; uint8_t out[372]{}; psp_writer_init(&writer,out,sizeof(out));
    CHECK(psp_payload_reading_serialize(&writer,nullptr)==PSP_INVALID_ARGUMENT);
    CHECK(psp_writer_status(&writer)==PSP_INVALID_ARGUMENT);
    CHECK(psp_writer_fail(&writer,PSP_BUFFER_TOO_SMALL)==PSP_INVALID_ARGUMENT);
    psp_reader reader; psp_reader_init(&reader,out,sizeof(out));
    CHECK(psp_payload_reading_deserialize(&reader,nullptr)==PSP_INVALID_ARGUMENT);
    CHECK(psp_reader_status(&reader)==PSP_INVALID_ARGUMENT);

    value.internal.sweeps[1].amu_id=2; value.internal.sweep_count=2;
    psp_writer_init(&writer,out,sizeof(out));
    CHECK(psp_payload_reading_serialize(&writer,&value)==PSP_INVALID_VALUE);
    CHECK(psp_writer_position(&writer)==0);
    value.internal.sweep_count=1; value.internal.sweeps[0].amu_id=31;
    psp_writer_init(&writer,out,sizeof(out));
    CHECK(psp_payload_reading_serialize(&writer,&value)==PSP_INVALID_VALUE);
    value.internal.sweeps[0].amu_id=2; value.internal.sweep_count=3;
    psp_writer_init(&writer,out,sizeof(out));
    CHECK(psp_payload_reading_serialize(&writer,&value)==PSP_INVALID_VALUE);
    value.internal.sweep_count=1; value.internal.sweeps=nullptr;
    psp_writer_init(&writer,out,sizeof(out));
    CHECK(psp_payload_reading_serialize(&writer,&value)==PSP_INVALID_VALUE);
}

int main() {
    test_golden_vector_and_truncation();
    test_float_bits_and_composition();
    test_capacity_ids_storage_and_mutators();
    test_short_writes_invalid_internal_state_and_sticky_errors();
    std::puts("PayloadReading checks passed");
}
