#include "frame/frame.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#define CHECK(x) do { if (!(x)) { std::fprintf(stderr,"failed line %d: %s\n",__LINE__,#x); std::exit(1); } } while(false)

static std::map<std::string, std::vector<uint8_t>> vectors() {
    std::ifstream input(std::string(PSP_TEST_SOURCE_DIR) + "/simulation/golden-vectors-v3.txt");
    CHECK(input.good());
    std::map<std::string, std::vector<uint8_t>> result;
    std::string key, hex;
    while (input >> key) {
        if (key[0] == '#') { std::getline(input, hex); continue; }
        input >> hex;
        std::vector<uint8_t> bytes;
        for (size_t i = 0; i < hex.size(); i += 2)
            bytes.push_back(static_cast<uint8_t>(std::stoul(hex.substr(i, 2), nullptr, 16)));
        result.emplace(key, bytes);
    }
    return result;
}
static void compare(const std::map<std::string,std::vector<uint8_t>>& golden,
                    const char *key, const uint8_t *actual, size_t size) {
    auto it = golden.find(key);
    CHECK(it != golden.end()); CHECK(it->second.size() == size);
    CHECK(std::memcmp(it->second.data(), actual, size) == 0);
}
int main() {
    const auto g = vectors();
    uint8_t bytes[PSP_FRAME_MAX_SERIALIZED_SIZE]; psp_writer w;

    psp_command command{}; command.kind=PSP_COMMAND_UTC_SYNC; command.request_id=0x01020304;
    command.body.utc_sync.offset_correction_ns=-1500; command.body.utc_sync.rate_ppb=-100000;
    psp_writer_init(&w,bytes,sizeof bytes); CHECK(psp_command_serialize(&w,&command)==PSP_OK);
    compare(g,"sync_command",bytes,psp_writer_position(&w));

    psp_sample sample{}; sample.kind=PSP_SAMPLE_IMU_STIMULUS; sample.body.imu_stimulus.sequence=7;
    psp_writer_init(&w,bytes,sizeof bytes); CHECK(psp_sample_serialize(&w,&sample)==PSP_OK);
    compare(g,"sample",bytes,psp_writer_position(&w));

    psp_log log{}; log.kind=PSP_LOG_UTC_REPORT; log.body.utc_report.id=0x01020304;
    log.body.utc_report.utc_at_receive_ns=0x0102030405060708ull;
    psp_writer_init(&w,bytes,sizeof bytes); CHECK(psp_log_serialize(&w,&log)==PSP_OK);
    compare(g,"utc_report_log",bytes,psp_writer_position(&w));

    log={}; log.kind=PSP_LOG_RESULT; log.body.result={0x01020304,2,PSP_RESULT_APPLIED};
    psp_writer_init(&w,bytes,sizeof bytes); CHECK(psp_log_serialize(&w,&log)==PSP_OK);
    compare(g,"result_log",bytes,psp_writer_position(&w));

    log={}; log.kind=PSP_LOG_STATUS; log.body.status={3,0x11223344,2,3,4,5};
    psp_writer_init(&w,bytes,sizeof bytes); CHECK(psp_log_serialize(&w,&log)==PSP_OK);
    compare(g,"status_log",bytes,psp_writer_position(&w));

    log={}; log.kind=PSP_LOG_I2C_TRANSACTION;
    auto &t=log.body.i2c_transaction; t.transaction_id=0x01020304; t.start_utc_ns=0x0102030405060708ull;
    t.t_read_offset_us=PSP_LOG_I2C_TRANSACTION_NO_READ; t.t_end_offset_us=300; t.flags=0x81;
    t.write_register=0x3b; t.write_length=1; t.write_data[0]=0x10; t.read_register=0x3b;
    t.read_length=2; t.read_data[0]=0x20; t.read_data[1]=0x21; t.sample_sequence=7; t.write_count=1; t.read_count=2;
    psp_writer_init(&w,bytes,sizeof bytes); CHECK(psp_log_serialize(&w,&log)==PSP_OK);
    compare(g,"transaction_log",bytes,psp_writer_position(&w));
    CHECK(PSP_FRAME_PREFIX_MAX_SERIALIZED_SIZE==20u); CHECK(PSP_FRAME_MAX_SERIALIZED_SIZE==87u);
    return 0;
}
