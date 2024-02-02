#include <stdint.h>
#include <stdlib.h>
#include <vitasdk.h>
#include <string.h>

#include "io.h"
#include "err.h"

int file_exist(char* path) {
	SceIoStat stat;
	int res = sceIoGetstat(path, &stat);
	if(res >= 0) return 1;
	else return 0;
}

int wait_for_partition(char* partiton) {
	for(int i = 0; i < 50; i++) {
		if(file_exist(partiton)) break;
		sceKernelDelayThread(5000);
	}
	return file_exist(partiton);
}

int get_files_in_folder(char* folder, char* out_filenames, size_t* total_folders, SearchFilter* filter, size_t max_files) {
	int ret = 0;
	
	// get total folder count
	*total_folders = 0;
	memset(out_filenames, 0x00, MAX_PATH * max_files);
	
	// read file list 
	int dfd = sceIoDopen(folder);
	sceClibPrintf("sceIoDopen dfd: %x\n", dfd);
	if(dfd < 0) ERROR(dfd);

	SceIoDirent ent;
	
	for(int i = 0; i < max_files; i++) {
		int res = sceIoDread(dfd, &ent);
		sceClibPrintf("sceIoDread res: %x\n", res);
		if(res < 0) ERROR(res);
		if(res == 0) break;
		if(ent.d_name == NULL) break;
		
		if(filter != NULL) {
			// ensure file is above a certain size
			if(ent.d_stat.st_size > filter->max_filesize) {
				sceClibPrintf("%s is too big\n", ent.d_name);
				continue;
			}
			
			// match only files
			if(filter->file_only && SCE_S_ISDIR(ent.d_stat.st_mode)) {
				sceClibPrintf("%s is directory\n", ent.d_name);
				continue;				
			}
			
			// match only specific file extension logic
			if(filter->match_extension[0] != '*') { 
				size_t dir_name_length = strlen(ent.d_name);
				size_t extension_length = strlen(filter->match_extension);

				sceClibPrintf("dir_name_length = %x\n", dir_name_length);
				sceClibPrintf("extension_length = %x\n", extension_length);
				
				if( strcasecmp (ent.d_name + (dir_name_length - extension_length), filter->match_extension) != 0 ) {
					sceClibPrintf("%s is not extension: %s\n", ent.d_name, filter->match_extension);
					continue;
				}
			}
			
		}
		sceClibPrintf("%s passed all filter checks\n", ent.d_name);
		
		strncpy(out_filenames + (*total_folders * MAX_PATH), ent.d_name, MAX_PATH-1); 					
		sceClibPrintf("ent.d_name: %s\n", ent.d_name);

		*total_folders += 1;
		sceClibPrintf("total_folders: %x\n", *total_folders);
	}
	
	ret = sceIoDclose(dfd);
	sceClibPrintf("sceIoDclose: %x\n", ret);
	if(ret > 0) ERROR(ret);
	
	*total_folders -= 1;
	
	return 0;
	error:
	sceClibPrintf("Error case reached: %x\n", ret);
	if(dfd >= 0)
		sceIoDclose(dfd);
	return ret;	
}

int read_first_filename(char* path, char* output, size_t out_size) {
	int ret = 0;
	int dfd = sceIoDopen(path);
	if(dfd < 0) ERROR(dfd);
	
	SceIoDirent ent;
	int res = sceIoDread(dfd, &ent);
	if(res < 0) ERROR(res);
	
	strncpy(output, ent.d_name, out_size);
	
error:
	if(dfd >= 0)
		sceIoDclose(dfd);
	return ret;	
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

uint64_t get_file_size(const char* filepath) {
	SceIoStat stat;
	int res = sceIoGetstat(filepath, &stat);
	if(res >= 0)
		return stat.st_size;
	return 0;
}

uint64_t get_free_space(const char* device) {
	uint64_t max_size = 0;
	uint64_t free_space = 0;
	 
	// host0 will always report as 0 bytes free
	if(strcmp("host0:", device) == 0)
		return 0xFFFFFFFFFFFFFFFF;
	 
	SceIoDevInfo info;
	int res = sceIoDevctl(device, 0x3001, NULL, 0, &info, sizeof(SceIoDevInfo));
	if (res < 0) {
		free_space = 0;
		max_size = 0;
	} else {
		max_size = info.max_size;
		free_space = info.free_size;
	}
	
	return free_space;
}
