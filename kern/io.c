#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <vitasdkkern.h>


int k_open_device(char* device, int permission) {
	int prev = ksceKernelSetPermission(0x80);
	int fd = ksceIoOpen(device, permission, 0777);
	ksceKernelSetPermission(prev);
	return fd;
}

int k_read_device(int device_handle, uint8_t* data, int size) {
	int prev = ksceKernelSetPermission(0x80);
	int res = ksceIoRead(device_handle, data, size);
	ksceKernelSetPermission(prev);
	return res;
}

int k_write_device(int device_handle, uint8_t* data, int size) {
	int prev = ksceKernelSetPermission(0x80);
	int res = ksceIoWrite(device_handle, data, size);
	ksceKernelSetPermission(prev);
	return res;
}

int k_close_device(int device_handle){
	int prev = ksceKernelSetPermission(0x80);
	ksceIoClose(device_handle);
	ksceKernelSetPermission(prev);
	return 0;
}

uint64_t k_get_device_size(int device_handle) {
	int prev = ksceKernelSetPermission(0x80);	
	uint64_t device_size = ksceIoLseek(device_handle, 0, SCE_SEEK_END);
	ksceIoLseek(device_handle, 0, SCE_SEEK_SET);
	ksceKernelSetPermission(prev);
	
	return device_size;
}

// io syscalls

int OpenDevice(char* device, int permission) {
	static char k_device[1028];
	
	ksceKernelStrncpyUserToKernel(k_device, (const void*)device, sizeof(k_device));
	
	return k_open_device(k_device, permission);
}


int WriteDevice(int device_handle, uint8_t* data, int size) {
	void* k_data = NULL;
	size_t k_size = 0;
	uint32_t k_offset = 0;
	
	int uid = ksceKernelUserMap("F00DBRIDGE_WRITE", 3, data, size, &k_data, &k_size, &k_offset);
	if(uid < 0)
		return uid;
	int res = k_write_device(device_handle, (k_data + k_offset) , size);
	ksceKernelMemBlockRelease(uid);
	
	return res;
}


int ReadDevice(int device_handle, uint8_t* data, int size) {
	void* k_data = NULL;
	size_t k_size = 0;
	uint32_t k_offset = 0;
	
	int uid = ksceKernelUserMap("F00DBRIDGE_READ", 3, data, size, &k_data, &k_size, &k_offset);
	if(uid < 0)
		return uid;
	int res = k_read_device(device_handle, (k_data + k_offset) , size);
	ksceKernelMemBlockRelease(uid);
	
	return res;
}

int CloseDevice(int device_handle) {
	return k_close_device(device_handle);
}

void GetDeviceSize(int device_handle, uint64_t* device_size) {
	uint64_t k_device_size = k_get_device_size(device_handle);
	ksceKernelMemcpyKernelToUser(device_size, (const void*)&k_device_size, sizeof(uint64_t));
}

