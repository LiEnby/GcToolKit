#include <stdint.h>

typedef struct sfo_header {
	char magic[4];
	uint32_t version;
	uint32_t key_offset;
	uint32_t value_offset;
	uint32_t count;
} sfo_header;

typedef struct sfo_key {
	uint16_t name_offset;
	uint8_t alignment;
	uint8_t type;
	uint32_t value_size;
	uint32_t total_size;
	uint32_t data_offset;
} sfo_key;

enum sfo_types {
	PSF_TYPE_BIN = 0,
	PSF_TYPE_STR = 2,
	PSF_TYPE_VAL = 4
};

int read_sfo_key(char* key, char* out, char* sfo);