#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vitasdk.h>

#include "aes.h"
#include "f00dbridge.h"
#include "crypto.h"

int key_dump(char* output_file) {
	GcKeys keys;
	
	int got_keys = extract_gc_keys(&keys);
	if(got_keys < 0) return got_keys;
	
	SceUID fd = sceIoOpen(output_file, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	sceClibPrintf("fd = %x\n", fd);
	if(fd < 0) return fd;
	
	int wr = sceIoWrite(fd, &keys, sizeof(GcKeys));
	sceClibPrintf("wr = %x (sizeof = %x)\n", wr, sizeof(GcKeys));
	sceIoClose(fd);

	if(wr == 0) return -1;
	if(wr < 0) return wr;
	if(wr != sizeof(GcKeys)) return (wr * -1);

	return 0;
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

void wait_for_gc_auth() {
	ResetCmd20Input();
	while(!HasCmd20Captured()) { sceKernelDelayThread(5000); };
}

int extract_gc_keys(GcKeys* keys) {
	if(HasCmd20Captured()) {
		// get captured cmd56 authentication data
		CommsData cmdData;
		GetLastCmd20Input(&cmdData);
		int keyId = GetLastCmd20KeyId();
		
		// decrypt secondaryKey0 (requires f00d)
		uint8_t secondaryKey0[0x10];
		memset(secondaryKey0, 0xFF, sizeof(secondaryKey0));
		DecryptSecondaryKey0(cmdData.packet6, keyId, cmdData.packet9, secondaryKey0);	

		// decrypt klic part from packet18
		decrypt_packet18_klic(secondaryKey0, cmdData.packet18, keys->klic);

		// decrypt rif part from packet20
		decrypt_packet20_rifbuf(secondaryKey0, cmdData.packet20, keys->rifbuf);
		return 0;
	}
	return -1;
}