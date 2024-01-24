#include "crypto.h"

#define BLOCK_DEVICE_GC "sdstor0:gcd-lp-ign-entire"
#define BLOCK_DEVICE_MEDIAID "sdstor0:gcd-lp-act-mediaid"
#define BLOCK_DEVICE_GRw0 "sdstor0:gcd-lp-ign-gamerw"

uint64_t device_size(char* block_device)
int dump_device(char* block_device, char* output_path, GcKeys* keys, void (*progress_callback)(char*, char*, uint64_t, uint64_t));
