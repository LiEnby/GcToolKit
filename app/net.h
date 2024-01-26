#include "io.h"
#include <stdint.h>

#define DEFAULT_IP "192.168.1.0"
#define DEFAULT_PORT 46327

#define SEND_FILE_MAGIC 38717

int init_network();
void term_network();
uint8_t is_connected();
uint8_t check_ip_address_valid(char* ip_address);

int begin_file_send(char* ip_address, unsigned short port, char* filename, uint64_t total_size);
int file_send_data(int fstream, void* data, size_t data_sz);
void end_file_send(int fstream);

typedef struct send_file_packet {
	uint16_t magic;
	char filename[MAX_PATH];
	uint64_t total_size;
} send_file_packet;