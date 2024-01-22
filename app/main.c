#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <taihen.h>
#include <vitasdk.h>

#include "graphics.h"
#include "ctrl.h"
#include "f00dbridge.h"

#include "aes.h"
#include "vci.h"

int kernel_started() {
	tai_module_info_t info;
	memset(&info, 0x00, sizeof(tai_module_info_t));
	
	info.size = sizeof(info);
	int res = taiGetModuleInfo("f00dbridge", &info);
	
	psvDebugScreenPrintf("Already Started f00d bridge... %x %x\n", res, info.modid);
	
	if(info.modid == 0x400100f5) 
		return 0;

	if(res >= 0) 
		return 0;
	
	return 0;
	
}

void init() {
	psvDebugScreenInit();
	
	char kplugin_path[0x200];
	memset(kplugin_path,0x00,0x200);
	
	if(kernel_started() == 0) {
		// get current title id
		char titleid[12];
		sceAppMgrAppParamGetString(0, 12, titleid , 256);
		//sceAppMgrUmount("app0:");

		// load psp2spl
		sprintf(kplugin_path, "ux0:app/%s/psp2spl.skprx", titleid);
		int psp2spl = taiLoadStartKernelModule(kplugin_path, 0, NULL, 0);
		psvDebugScreenPrintf("Loading %s ... %x\n", kplugin_path, psp2spl);
		
		// load f00dbridge
		sprintf(kplugin_path, "ux0:app/%s/kplugin.skprx", titleid);
		int kplugin = taiLoadStartKernelModule(kplugin_path, 0, NULL, 0);
		psvDebugScreenPrintf("Loading %s ... %x\n", kplugin_path, kplugin);		
		if(kplugin < 0) return;
		
		// restart this application
		sprintf(kplugin_path, "app0:/eboot.bin", titleid);
		psvDebugScreenPrintf("Restarting: %s ... ", kplugin_path);
		int res = sceAppMgrLoadExec(kplugin_path, NULL, NULL);
		psvDebugScreenPrintf("%x\n", res);
		
	}
	
}

int print_buffer(uint8_t* buffer, size_t buffer_size){
	for(int i = 0; i < buffer_size; i++)
		psvDebugScreenPrintf("%02X", buffer[i]);
	psvDebugScreenPrintf("\n");
}

void decrypt(uint8_t* masterKey, uint8_t* data, size_t dataLen) {
	char iv[0x10];
	memset(iv, 0x00, sizeof(iv));
	
	struct AES_ctx ctx;
	AES_init_ctx_iv(&ctx, masterKey, iv);
	AES_CBC_decrypt_buffer(&ctx, data, dataLen);
}


void decrypt_packet18_klic(uint8_t* secondaryKey0, uint8_t* packet18, uint8_t* klic) {
	char wbuf[0x30];
	memcpy(wbuf, packet18+3, sizeof(wbuf));
	decrypt(secondaryKey0, wbuf, sizeof(wbuf));
	
	memcpy(klic, wbuf+0x10, 0x20);
}

void decrypt_packet20_rifbuf(uint8_t* secondaryKey0, uint8_t* packet20, uint8_t* rifBuffer) {
	char wbuf[0x40];
	memcpy(wbuf, packet20+3, sizeof(wbuf));
	decrypt(secondaryKey0, wbuf, sizeof(wbuf));

	memcpy(rifBuffer, wbuf+0x18, 0x20);

}

void extract_gc_keys(char* keys) {
	
	psvDebugScreenPrintf("Get comms data ...\n");

	// get captured cmd56 authentication data
	CommsData cmdData;
	GetLastCmd20Input(&cmdData);
	int keyId = GetLastCmd20KeyId();
	
	// decrypt secondaryKey0 (requires f00d)
	uint8_t secondaryKey0[0x10];
	memset(secondaryKey0, 0xFF, sizeof(secondaryKey0));
	DecryptSecondaryKey0(cmdData.packet6, keyId, cmdData.packet9, secondaryKey0);
	
	psvDebugScreenPrintf("Get secondaryKey0 ... ");		
	print_buffer(secondaryKey0, sizeof(secondaryKey0));

	// decrypt klic part from packet18
	char klic[0x20];
	decrypt_packet18_klic(secondaryKey0, cmdData.packet18, klic);

	psvDebugScreenPrintf("Get Key Parital 1 ... ");		
	print_buffer(klic, sizeof(klic));

	// decrypt rif part from packet20
	char rif[0x20];
	decrypt_packet20_rifbuf(secondaryKey0, cmdData.packet20, rif);
	
	psvDebugScreenPrintf("Get Key Parital 2 ... ");		
	print_buffer(rif, sizeof(rif));

	// extract keys
	memset(keys, 0x00, 0x40);
	memcpy(keys, klic, 0x20);
	memcpy(keys+0x20, rif, 0x20);
	
}

int dump_gc(char* output_path, char* keys) {
	char* GC_DEVICE = "sdstor0:gcd-lp-ign-entire";
	SceUID gc_fd = sceIoOpen(output_path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	
	
	psvDebugScreenPrintf("Begin Device Dump ...\n");
	
	int device_fd = OpenDevice(GC_DEVICE);
	psvDebugScreenPrintf("Open Device: %s = 0x%04X\n", GC_DEVICE, device_fd);
	
	uint64_t device_size = 0;
	GetDeviceSize(device_fd, &device_size);
	psvDebugScreenPrintf("Device Size: 0x%llx\n", device_size);
	
	// write VCI file
	psvDebugScreenPrintf("Write file header ...\n");
	
	VciFile vci;
	memset(&vci, 0x00, sizeof(VciFile));
	strncpy(vci.magic, "VCI", sizeof(vci.magic));
	vci.version = 1;
	vci.devicesize = device_size;
	memcpy(vci.key1, keys, sizeof(vci.key1) * 2);
	
	sceIoWrite(gc_fd, &vci, sizeof(VciFile)); // write header
	
	
	uint64_t total_read = 0;

	do {
		static uint8_t data[0x20000]__attribute__((aligned(0x40))); // exfatfs does each 0x20000 reading internally - Princess of Sleeping 
		int res = ReadDevice(device_fd, data, sizeof(data)); // read raw from device
		if(res < 0) { psvDebugScreenPrintf("ReadDevice error : %x\n", res); break; }
		
		total_read += sceIoWrite(gc_fd, data, res); // write raw data
		
		psvDebugScreenPrintf("Written: 0x%llx / 0x%llx ... %i%%\n", total_read, device_size, (int)(((float)total_read / (float)device_size)*100.0));
	} while(total_read < device_size);
	
	sceIoClose(gc_fd);
	CloseDevice(device_fd);
	
	return 0;
}


int main() {

	init();
	psvDebugScreenPrintf("Please insert a GameCart!\n");
	psvDebugScreenPrintf("(If theres already one, eject it and put it back in)\n");

	while(!HasCmd20Captured()) { sceKernelDelayThread(5000); };

	char keys[0x40];
	extract_gc_keys(keys);
	dump_gc("uma0:/GC.VCI", keys);
	
	get_key();
	
	return 0;
}
