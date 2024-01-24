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

// exfatfs does each 0x20000 reading internally - Princess of Sleeping 
static uint8_t DEVICE_DUMP_BUFFER[0x20000]__attribute__((aligned(0x40))); 

uint64_t device_size(char* block_device) {

	uint64_t device_size = 0;

	int dfd = OpenDevice(block_device);
	if(dfd < 0)
		return 0;
	GetDeviceSize(dfd, &device_size);
	CloseDevice(dfd);
	
	return device_size;
	
}

int dump_device(char* block_device, char* output_path, GcKeys* keys, void (*progress_callback)(char*, char*, uint64_t, uint64_t)) {
	int ret = 0;
	
	sceClibPrintf("Begining dump of %s to %s\n", block_device, output_path);
	
	// open image file
	SceUID gc_fd = sceIoOpen(output_path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	sceClibPrintf("gc_fd = %x\n", gc_fd);
	if(gc_fd < 0) ERROR(gc_fd);

	// open gc
	int device_fd = OpenDevice(block_device);
	sceClibPrintf("device_fd = %x\n", device_fd);
	if(device_fd < 0) ERROR(device_fd);
	
	// get gc size
	uint64_t device_size = 0;
	GetDeviceSize(device_fd, &device_size);
	sceClibPrintf("device_size = %x\n", device_size);
	if(device_size == 0) ERROR(-1);
	
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
	uint64_t total_read = 0;
	if(progress_callback != NULL) progress_callback(block_device, output_path, total_read, device_size);


	do {
		int rd = ReadDevice(device_fd, DEVICE_DUMP_BUFFER, sizeof(DEVICE_DUMP_BUFFER)); // read raw from device
		if(rd == 0) ERROR(-2);
		if(rd < 0) ERROR(rd);

		int wr = sceIoWrite(gc_fd, DEVICE_DUMP_BUFFER, rd); // write raw data
		if(wr == 0) ERROR(-3);
		if(wr < 0) ERROR(wr);
		
		total_read += wr;
		sceKernelPowerTick(SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND);
		if(progress_callback != NULL) progress_callback(block_device, output_path, total_read, device_size);
	} while(total_read < device_size);
	
error:
	if(gc_fd >= 0)
		sceIoClose(gc_fd);
	if(device_fd >= 0)
		CloseDevice(device_fd);
	
	return ret;
}