#include <stdint.h>
#include <vitasdk.h>
#include <string.h>

#include "io.h"

int has_gro0() {
	return file_exist("gro0:");	
}

int has_grw0() {
	return file_exist("grw0:");	
}

int file_exist(char* path) {
	SceIoStat stat;
	int res = sceIoGetstat(path, &stat);
	if(res >= 0) return 1;
	else return 0;
}

int wait_for_partition(char* partiton) {
	for(int i = 0; i < 300; i++) {
		if(file_exist(partiton)) break;
		sceKernelDelayThread(10000);
	}
	return file_exist(partiton);
}

void remove_illegal_chars(char* str) {
	// remove illegal characters from file name
	int slen = strlen(str);
	for(int i = 0; i < slen; i++) {
		if(str[i] == '/' ||
		str[i] == '\\' ||
		str[i] == ':' ||
		str[i] == '?' ||
		str[i] == '*' ||
		str[i] == '"' ||
		str[i] == '|' ||
		str[i] == '>' ||
		str[i] == '\n' ||
		str[i] == '\r' ||
		str[i] == '<')
			str[i] = ' ';		
	}
}

int mount_partition(int id, const char *path, int permission, int a4, int a5, int a6) {
  uint32_t buf[3];

  buf[0] = a4;
  buf[1] = a5;
  buf[2] = a6;

  return _vshIoMount(id, path, permission, buf);
}

int mount_uma() {
	return mount_partition(0xF00, NULL, 2, 0, 0, 0); // mount uma0
}

int mount_xmc() {
	return mount_partition(0xE00, NULL, 2, 0, 0, 0); // mount xmc
}

int mount_imc() {
	return mount_partition(0xD00, NULL, 2, 0, 0, 0); // mount imc
}

int mount_gro0() {
	return mount_partition(0x900, NULL, 1, 0, 0, 0); // mount gro0
}

int mount_grw0() {
	return mount_partition(0xA00, NULL, 2, 0, 0, 0); // mount grw0
}

void umount_gro0() {
	vshIoUmount(0x900, 0, 0, 0);
	vshIoUmount(0x900, 1, 0, 0);
}

void umount_grw0() {
	vshIoUmount(0xA00, 0, 0, 0);
	vshIoUmount(0xA00, 1, 0, 0);
}

void umount_uma() {
	vshIoUmount(0xF00, 0, 0, 0);
	vshIoUmount(0xF00, 1, 0, 0);
}

void umount_xmc() {
	vshIoUmount(0xE00, 0, 0, 0);
	vshIoUmount(0xE00, 1, 0, 0);
}

void umount_imc() {
	vshIoUmount(0xD00, 0, 0, 0);
	vshIoUmount(0xD00, 1, 0, 0);
}

void umount_devices() {
	umount_uma();
	umount_xmc();		
}

void mount_devices() {
	int res = mount_uma();
	if(res >= 0) wait_for_partition("uma0:");
	
	res = mount_xmc();
	if(res >= 0) wait_for_partition("xmc0:");	
}

