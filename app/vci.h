#ifndef VCI_H 
#define VCI_H

#include <stdint.h>
#include "crypto.h"

#define VCI_HDR "VCI\0"

typedef struct VciFile {
	char magic[0x4];
	int version;
	uint64_t devicesize;
	GcKEYS keys;
	uint8_t padding[0x1B0];
} VciFile;


#endif