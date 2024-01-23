#include "sfo.h"
#include <vitasdk.h>
#include <string.h>
#include <stdio.h>

static uint8_t SFO_BUFFER[0x10000];

int read_sfo_key(char* key, char* out, char* sfo) {
	memset(SFO_BUFFER, 0x00, sizeof(SFO_BUFFER));
	// open sfo file
	int sfo_fd = sceIoOpen(sfo, SCE_O_RDONLY, 0777);	
	if(sfo_fd < 0) return sfo_fd;

	// read sfo
	int sfo_size = sceIoRead(sfo_fd, SFO_BUFFER, sizeof(SFO_BUFFER));
	// close sfo
	sceIoClose(sfo_fd);

	if(sfo_size < 0) return sfo_size;
	if(sfo_size >= sizeof(SFO_BUFFER)) return -1;
	if(sfo_size <= sizeof(sfo_header)) return -2;
	
	
	// get sfo header
	sfo_header header;
	memcpy(&header, SFO_BUFFER, sizeof(sfo_header));
	
	if(memcmp(header.magic, "\0PSF", sizeof(header.magic)) != 0) return -3; // check magic	
	if(header.count > 200) return -4; // give up if more than 200 keys
	if(sfo_size < (sizeof(sfo_header) + (sizeof(sfo_key) * header.count))) return -5; // check if size is enough for keys + sfo header size
	
	uint32_t ptr = sizeof(sfo_header);
	
	// read keys
	for(int i = 0; i < header.count; i++)
	{
		if(ptr >= sizeof(SFO_BUFFER)) return -6; // check for overflow

		char key_name[64];
		char key_value[64];
		
		sfo_key s_key;
		
		memcpy(&s_key, SFO_BUFFER+ptr, sizeof(sfo_key));
		ptr += sizeof(sfo_key);

		if(s_key.type != PSF_TYPE_STR) continue;
		
		// calculate location of key in buffer
		int name_offset = header.key_offset + s_key.name_offset;
		if(name_offset > sfo_size) return -7;
		
		int data_offset = header.value_offset + s_key.data_offset;
		if(data_offset > sfo_size) return -8;

		// copy the key and value into buffers.
		strncpy(key_name, SFO_BUFFER + name_offset, sizeof(key_name)-1);
		strncpy(key_value, SFO_BUFFER + data_offset, sizeof(key_value)-1);		

		if(strncmp(key_name, key, sizeof(key_name)) == 0){
			strncpy(out, key_value, sizeof(key_value)-1);
			return 0;
		}

	}
	return -9;
}