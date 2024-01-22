#include <stdint.h>

typedef struct VciFile {
	char magic[0x4];
	int version;
	uint64_t devicesize;
	uint8_t key1[0x20];
	uint8_t key2[0x20];
	uint8_t padding[0x1B0];
} VciFile;