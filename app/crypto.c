#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vitasdk.h>

#include "aes.h"
#include "sha256.h"
#include "sha1.h"

#include "f00dbridge.h"
#include "crypto.h"
#include "net.h"
#include "err.h"

int key_dump_network(char* ip_address, unsigned short port, char* output_file) {
	GcKeys keys;
	
	int got_keys = extract_gc_keys(&keys);
	if(got_keys < 0) return got_keys;
	
	SceUID fd = begin_file_send(ip_address, port, output_file, sizeof(GcKeys));
	sceClibPrintf("fd = %x\n", fd);
	if(fd < 0) return fd;
	
	int wr = file_send_data(fd, &keys, sizeof(GcKeys));
	sceClibPrintf("wr = %x (sizeof = %x)\n", wr, sizeof(GcKeys));
	end_file_send(fd);

	if(wr == 0) return -1;
	if(wr < 0) return wr;
	if(wr != sizeof(GcKeys)) return (wr * -1);

	return 0;
}

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

void print_buffer(char* buffer, int size){
	for(int i = 0; i < size; i++) {
		sceClibPrintf("%02X ", (unsigned char)(buffer[i]));		
	}
	sceClibPrintf("\n");
}

uint8_t verify_klic_keys(GcKeys* keys) {
	char got_final_keys[SHA256_BLOCK_SIZE];
	char expected_final_keys[SHA256_BLOCK_SIZE];
	int res = GetFinalKeys(got_final_keys);
	if(res < 0) goto error;
	
	// final key should == sha256 of klic key
	SHA256_CTX ctx;
	
	sha256_init(&ctx);
	sha256_update(&ctx, keys, sizeof(GcKeys));
	sha256_final(&ctx, expected_final_keys);
	
	if(memcmp(got_final_keys, expected_final_keys, SHA256_BLOCK_SIZE) == 0) {
		return 1;
	}
error:
	// print info
	sceClibPrintf("GcKeys ");
	print_buffer(keys, sizeof(GcKeys));
	
	sceClibPrintf("verify_klic_keys failed !\n");
	sceClibPrintf("got_final_keys ");
	print_buffer(got_final_keys, sizeof(got_final_keys));
	
	sceClibPrintf("expected_final_keys ");
	print_buffer(expected_final_keys, sizeof(expected_final_keys));
	
	return 0;
}

uint8_t verify_rif_keys(GcKeys* keys) {
	int ret = 0;
	char expected_final_rif_hash[SHA1_BLOCK_SIZE];
	char got_final_rif_hash[SHA1_BLOCK_SIZE];

	SHA1_CTX ctx;
	
	sha1_init(&ctx);
	sha1_update(&ctx, keys->rifbuf, sizeof(keys->rifbuf));
	sha1_final(&ctx, expected_final_rif_hash);
	
	// get title id from license folder
	char folder[MAX_PATH];
	snprintf(folder, MAX_PATH, "gro0:/license/app");
	sceClibPrintf("folder = %s\n", folder);
	
	char TITLE_ID[12];
	int res = read_first_filename(folder, TITLE_ID, sizeof(TITLE_ID));
	sceClibPrintf("read_first_filename license folder res = 0x%x\n",res);
	if(res < 0) ERROR(-1);

	sceClibPrintf("TITLE_ID = %s\n", TITLE_ID);
	snprintf(folder, MAX_PATH, "gro0:/license/app/%s", TITLE_ID);
	sceClibPrintf("folder = %s\n", folder);
	
	// get rif name from license/titleid folder
	char RIF_NAME[0x50];
	res = read_first_filename(folder, RIF_NAME, sizeof(RIF_NAME));
	sceClibPrintf("read_first_filename license titleid folder res = 0x%x\n",res);
	if(res < 0) ERROR(-2);
	
	sceClibPrintf("RIF_NAME = %s\n", RIF_NAME);
	snprintf(folder, MAX_PATH, "gro0:/license/app/%s/%s", TITLE_ID, RIF_NAME);
	
	sceClibPrintf("folder = %s\n", folder);
	
	// read the hash from the rif file ..
	
	SceUID fd = sceIoOpen(folder, SCE_O_RDONLY, 0777);
	sceClibPrintf("rif fd = %x\n", fd);
	if(fd < 0) ERROR(-3);
	
	uint64_t loc = sceIoLseek(fd, 0xE0, SCE_SEEK_SET);
	sceClibPrintf("sceIoLseek loc = %llx\n", loc);
	if(loc != 0xE0) ERROR(-4);
	
	res = sceIoRead(fd, got_final_rif_hash, SHA1_BLOCK_SIZE);
	sceClibPrintf("sceIoRead res = %x\n", res);
	if(res != SHA1_BLOCK_SIZE) ERROR(-5);
	
	sceIoClose(fd);
	
	// final rif hash should == 0xe0 in rif file
	
	if(memcmp(expected_final_rif_hash, got_final_rif_hash, SHA1_BLOCK_SIZE) == 0){
		return 1;
	}

	sceClibPrintf("verify_rif_keys failed!\n");
	
	return 0;
error:	
	if(fd > 0)
		sceIoClose(fd);
	return 0;
	
}

int extract_gc_keys(GcKeys* keys) {
	if(HasCmd20Captured()) {
		// get captured cmd56 authentication data
		CommsData cmdData;
		GetLastCmd20Input(&cmdData);
		int keyId = GetLastCmd20KeyId();
		sceClibPrintf("keyId = %x\n", keyId);
		
		// decrypt secondaryKey0 (requires f00d)
		uint8_t secondaryKey0[0x10];
		memset(secondaryKey0, 0xFF, sizeof(secondaryKey0));
		DecryptSecondaryKey0(cmdData.packet6, keyId, cmdData.packet9, secondaryKey0);	
	
		sceClibPrintf("secondaryKey0 ");
		print_buffer(secondaryKey0, sizeof(secondaryKey0));
			
		// decrypt klic part from packet18
		decrypt_packet18_klic(secondaryKey0, cmdData.packet18, keys->klic);

		// decrypt rif part from packet20
		decrypt_packet20_rifbuf(secondaryKey0, cmdData.packet20, keys->rifbuf);
		
		// verify klic key
		if(!verify_klic_keys(keys)) return -3;
		
		// verify rif buffer
		if(has_gro0())
			if(!verify_rif_keys(keys)) return -2;

		
		return 0;
	}
	return -1;
}