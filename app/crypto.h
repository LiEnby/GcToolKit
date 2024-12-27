#include "mbr.h"
int key_dump_network(char* ip_address, unsigned short port, char* output_file);
int key_dump(char* output_file);
void wait_for_gc_auth();
int extract_gc_keys(GcKEYS* keys);
