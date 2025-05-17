#include "crypto.h"

#define BLOCK_DEVICE_GC "sdstor0:gcd-lp-ign-entire"
#define BLOCK_DEVICE_MEDIAID "sdstor0:gcd-lp-act-mediaid"
#define BLOCK_DEVICE_GRW0 "sdstor0:gcd-lp-ign-gamerw"
#define BLOCK_DEVICE_GRO0 "sdstor0:gcd-lp-ign-gamero"


uint64_t device_size(const char* block_device);
int dump_device_network(char* ip_address, unsigned short port, const char* block_device, char* output_path, GcCmd56Keys* keys, void (*progress_callback)(const char*, char*, uint64_t, uint64_t));
int dump_device(const char* block_device, char* output_path, GcCmd56Keys* keys, void (*progress_callback)(const char*, char*, uint64_t, uint64_t));
int wipe_device(const char* block_device, void (*progress_callback)(const char*, char*, uint64_t, uint64_t));
int restore_device(const char* block_device, char* input_path, void (*progress_callback)(const char*, char*, uint64_t, uint64_t));
uint8_t device_exist(const char* block_device);