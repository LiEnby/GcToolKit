#ifndef CRYPTO_H
#define CRYPTO_H 1

typedef struct GcKeys{
	uint8_t rifbuf[0x20];
	uint8_t klic[0x20];
} GcKeys;

int key_dump_network(char* ip_address, unsigned short port, char* output_file);
int key_dump(char* output_file);
void wait_for_gc_auth();
int extract_gc_keys(GcKeys* keys);

#endif