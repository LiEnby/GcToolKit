#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vitasdk.h>

#include "f00dbridge.h"
#include "device.h"
#include "crypto.h"
#include "vci.h"
#include "err.h"
#include "net.h"

// exfatfs does each 0x20000 reading internally - Princess of Sleeping 
static uint8_t DEVICE_DUMP_BUFFER[0x20000]__attribute__((aligned(0x40))); 

uint8_t device_exist(char* block_device) {
	int dfd = OpenDevice(block_device, SCE_O_RDONLY);
	if(dfd < 0)
		return 0;
	CloseDevice(dfd);
	return 1;
}

uint64_t device_size(char* block_device) {

	uint64_t device_size = 0;

	int dfd = OpenDevice(block_device, SCE_O_RDONLY);
	if(dfd < 0)
		return 0;
	GetDeviceSize(dfd, &device_size);
	CloseDevice(dfd);
	
	return device_size;
	
}
int dump_device_network(char* ip_address, unsigned short port, char* block_device, char* output_path, GcKeys* keys, void (*progress_callback)(char*, char*, uint64_t, uint64_t)) {
	int ret = 0;	
	uint64_t total_read = 0;
	
	sceClibPrintf("Begining NETWORK dump of %s to %s:%u\n", block_device, ip_address, port);
	
	// open gc
	int device_fd = OpenDevice(block_device, SCE_O_RDONLY);
	sceClibPrintf("device_fd = %x\n", device_fd);
	if(device_fd < 0) ERROR(device_fd);
	
	// get gc size
	uint64_t device_size = 0;
	GetDeviceSize(device_fd, &device_size);
	sceClibPrintf("device_size = %llx\n", device_size);
	if(device_size == 0) ERROR(-1);
	
	if(progress_callback != NULL) progress_callback(block_device, output_path, total_read, device_size);

	// open socket
	SceUID gc_sock = begin_file_send(ip_address, port, output_path, (keys != NULL) ? device_size + sizeof(VciFile) : device_size);
	sceClibPrintf("gc_sock = %x\n", gc_sock);
	if(gc_sock < 0) ERROR(gc_sock);

	
	if(keys != NULL) {
		// generate VCI header
		VciFile vci;
		memset(&vci, 0x00, sizeof(VciFile));
		
		memcpy(vci.magic, VCI_HDR, sizeof(vci.magic));
		vci.version = 1;
		vci.devicesize = device_size;
		memcpy(&vci.keys, keys, sizeof(GcKeys));
		
		// write VCI header to file
		int wr = file_send_data(gc_sock, &vci, sizeof(VciFile));
		sceClibPrintf("wr = %x\n", wr);
		
		if(wr == 0) ERROR(-1);
		if(wr != sizeof(VciFile)) ERROR(wr * -1);
		if(wr < 0) ERROR(wr);
	}

	// enter read/write loop
	do {
		int rd = ReadDevice(device_fd, DEVICE_DUMP_BUFFER, sizeof(DEVICE_DUMP_BUFFER)); // read raw from device
		if(rd == 0) ERROR(-2);
		if(rd < 0) ERROR(rd);

		int wr = file_send_data(gc_sock, DEVICE_DUMP_BUFFER, rd); // send raw data
		if(wr == 0) ERROR(-3);
		if(wr < 0) ERROR(wr);
		
		total_read += wr;
		if(progress_callback != NULL) progress_callback(block_device, output_path, total_read, device_size);
	} while(total_read < device_size);
	
error:
	if(gc_sock >= 0)
		end_file_send(gc_sock);
	if(device_fd >= 0)
		CloseDevice(device_fd);
	
	return ret;
}

int dump_device(char* block_device, char* output_path, GcKeys* keys, void (*progress_callback)(char*, char*, uint64_t, uint64_t)) {
	int ret = 0;
	uint64_t total_read = 0;

	sceClibPrintf("Begining dump of %s to %s\n", block_device, output_path);
	
	// open image file
	SceUID gc_fd = sceIoOpen(output_path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	sceClibPrintf("gc_fd = %x\n", gc_fd);
	if(gc_fd < 0) ERROR(gc_fd);

	// open device
	int device_fd = OpenDevice(block_device, SCE_O_RDONLY);
	sceClibPrintf("device_fd = %x\n", device_fd);
	if(device_fd < 0) ERROR(device_fd);
	
	// get device size
	uint64_t device_size = 0;
	GetDeviceSize(device_fd, &device_size);
	sceClibPrintf("device_size = %llx\n", device_size);
	if(device_size == 0) ERROR(-1);
	
	if(progress_callback != NULL) progress_callback(block_device, output_path, total_read, device_size);

	if(keys != NULL) {
		// generate VCI header
		VciFile vci;
		memset(&vci, 0x00, sizeof(VciFile));
		
		memcpy(vci.magic, VCI_HDR, sizeof(vci.magic));
		vci.version = 1;
		vci.devicesize = device_size;
		memcpy(&vci.keys, keys, sizeof(GcKeys));
		
		// write VCI header to file
		int wr = sceIoWrite(gc_fd, &vci, sizeof(VciFile));
		sceClibPrintf("wr = %x\n", wr);
		
		if(wr == 0) ERROR(-1);
		if(wr != sizeof(VciFile)) ERROR(wr * -1);
		if(wr < 0) ERROR(wr);		
	}

	// enter read/write loop
	do {
		int rd = ReadDevice(device_fd, DEVICE_DUMP_BUFFER, sizeof(DEVICE_DUMP_BUFFER)); // read raw from device
		if(rd == 0) ERROR(-2);
		if(rd < 0) ERROR(rd);

		int wr = sceIoWrite(gc_fd, DEVICE_DUMP_BUFFER, rd); // write raw data
		if(wr == 0) ERROR(-3);
		if(wr < 0) ERROR(wr);
		
		total_read += wr;
		if(progress_callback != NULL) progress_callback(block_device, output_path, total_read, device_size);
	} while(total_read < device_size);
	
error:
	if(gc_fd >= 0)
		sceIoClose(gc_fd);
	if(device_fd >= 0)
		CloseDevice(device_fd);
	
	return ret;
}

int restore_device(char* block_device, char* input_path, void (*progress_callback)(char*, char*, uint64_t, uint64_t)) {
	int ret = 0;
	uint64_t total_write = 0;
	
	// sanity safety check
	if(memcmp(block_device, "sdstor0:gcd", strlen("sdstor0:gcd")) != 0)
		return -1; 
	
	sceClibPrintf("Begining restore of %s to %s\n", input_path, block_device);
	
	// open image file
	SceUID gc_fd = sceIoOpen(input_path, SCE_O_RDONLY, 0777);
	sceClibPrintf("gc_fd = %x\n", gc_fd);
	if(gc_fd < 0) ERROR(gc_fd);

	// open device
	int device_fd = OpenDevice(block_device, SCE_O_WRONLY);
	sceClibPrintf("device_fd = %x\n", device_fd);
	if(device_fd < 0) ERROR(device_fd);
	
	// get device size
	uint64_t device_size = 0;
	GetDeviceSize(device_fd, &device_size);
	sceClibPrintf("device_size = %llx\n", device_size);
	if(device_size == 0) ERROR(-1);
	
	if(progress_callback != NULL) progress_callback(block_device, input_path, total_write, device_size);

	// enter read/write loop
	do {
		int rd = sceIoRead(gc_fd, DEVICE_DUMP_BUFFER, sizeof(DEVICE_DUMP_BUFFER)); // Read img data
		if(rd == 0) ERROR(-3);
		if(rd < 0) ERROR(rd);

		int wr = WriteDevice(device_fd, DEVICE_DUMP_BUFFER, rd); // Write raw data to device
		if(wr == 0) ERROR(-2);
		if(wr < 0) ERROR(wr);

		
		total_write += wr;
		if(progress_callback != NULL) progress_callback(block_device, input_path, total_write, device_size);
	} while(total_write < device_size);
	
error:
	if(gc_fd >= 0)
		sceIoClose(gc_fd);
	if(device_fd >= 0)
		CloseDevice(device_fd);
	
	return ret;
}

int wipe_device(char* block_device, void (*progress_callback)(char*, char*, uint64_t, uint64_t)) {
	int ret = 0;
	uint64_t total_write = 0;

	// sanity safety check
	if(memcmp(block_device, "sdstor0:gcd", strlen("sdstor0:gcd")) != 0)
		return -1; 
		
	sceClibPrintf("Begining wipe of %s\n", block_device);
	
	// open device
	int device_fd = OpenDevice(block_device, SCE_O_WRONLY); // SCARY
	sceClibPrintf("device_fd = %x\n", device_fd);
	if(device_fd < 0) ERROR(device_fd);
	
	// get device size
	uint64_t device_size = 0;
	GetDeviceSize(device_fd, &device_size);
	sceClibPrintf("device_size = %llx\n", device_size);
	if(device_size == 0) ERROR(-1);
	
	if(progress_callback != NULL) progress_callback(block_device, block_device, total_write, device_size);
	memset(DEVICE_DUMP_BUFFER, 0x00, sizeof(DEVICE_DUMP_BUFFER));

	// enter write loop
	do {
		int wr = WriteDevice(device_fd, DEVICE_DUMP_BUFFER, sizeof(DEVICE_DUMP_BUFFER)); // write raw to device
		if(wr == 0) ERROR(-2);
		if(wr < 0) ERROR(wr);
		
		total_write += wr;
		
		if(progress_callback != NULL) progress_callback(block_device, block_device, total_write, device_size);
	} while(total_write < device_size);
	
error:
	if(device_fd >= 0)
		CloseDevice(device_fd);
	
	return ret;
}