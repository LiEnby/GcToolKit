#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vitasdk.h>

#include "aes.h"
#include "aes_cmac.h"

#include "sha256.h"
#include "sha1.h"

#include "io.h"
#include "device.h"

#include "f00dbridge.h"
#include "crypto.h"
#include "net.h"
#include "err.h"
#include "log.h"

static uint8_t BIGMAC_KEY_0x345[0x20] = { 0x74, 0xC3, 0x9C, 0xA4, 0xEF, 0x4F, 0x12, 0x29, 0x15, 0xC7, 0x1E, 0xDA, 0x46, 0xC8, 0x8B, 0x55, 0xBB, 0xAD, 0x1F, 0x40, 0x33, 0xD7, 0x55, 0xCE, 0xA0, 0x56, 0x3C, 0xC3, 0x41, 0xF9, 0x2E, 0x66 };
static uint8_t BIGMAC_KEY_0x348[0x20] = { 0xC0, 0x26, 0x28, 0x14, 0x13, 0xFA, 0x46, 0x2C, 0xCD, 0xEE, 0xD4, 0xBD, 0x6D, 0x08, 0xC3, 0x7C, 0xA6, 0xC9, 0x32, 0x2A, 0xBD, 0x4C, 0x40, 0xAD, 0xE7, 0x2A, 0x0F, 0x54, 0x4F, 0x40, 0x13, 0xAD };

static uint8_t GCAUTHMGR_8001_SEED[0x10] = { 0x6f, 0x22, 0x85, 0xed, 0x46, 0x3a, 0x6e, 0x57, 0xc5, 0xf3, 0x55, 0x0d, 0xdc, 0xc8, 0x1f, 0xeb };
static uint8_t GCAUTHMGR_8002_SEED[0x10] = { 0xda, 0x96, 0x08, 0xb5, 0x28, 0x82, 0x5d, 0x6d, 0x13, 0xa7, 0xaf, 0x14, 0x46, 0xb8, 0xec, 0x08 };
static uint8_t GCAUTHMGR_8003_SEED[0x10] = { 0x36, 0x8b, 0x2e, 0xb5, 0x43, 0x7a, 0x82, 0x18, 0x62, 0xa6, 0xc9, 0x55, 0x96, 0xd8, 0xc1, 0x35 };
static uint8_t GCAUTHMGR_1_SEED[0x10] 	 = { 0x7f, 0x1f, 0xd0, 0x65, 0xdd, 0x2f, 0x40, 0xb3, 0xe2, 0x65, 0x79, 0xa6, 0x39, 0x0b, 0x61, 0x6d };

static uint8_t GCAUTHMGR_1_IV[0x10] = { 0x8b, 0x14, 0xc8, 0xa1, 0xe9, 0x6f, 0x30, 0xa7, 0xf1, 0x01, 0xa9, 0x6a, 0x30, 0x33, 0xc5, 0x5b };
static uint8_t ZERO_IV[0x10] = { 0 };

void derive_master_key(uint8_t* cartRandom, uint8_t* masterkey, int keyId) {
	uint8_t* kseed = NULL;
	uint8_t x21[0x10];
	uint8_t cmac[0x10];
	
	switch (keyId) {
	case 0x8001:
		kseed = GCAUTHMGR_8001_SEED;
		break;
	case 0x8002:
		kseed = GCAUTHMGR_8002_SEED;
		break;
	case 0x8003:
		kseed = GCAUTHMGR_8003_SEED;
		break;
	case 0x1:
		kseed = GCAUTHMGR_1_SEED;
		break;
	}

	AES_ECB_decrypt(kseed, x21, sizeof(x21), BIGMAC_KEY_0x345, 0x20);
	aes_cmac(cartRandom, 0x20, x21, cmac);
	
	if (keyId == 0x1) {
		AES_CBC_decrypt(cmac, masterkey, 0x10, BIGMAC_KEY_0x348, 0x10, GCAUTHMGR_1_IV);
	}
	else {
		memcpy(masterkey, cmac, sizeof(cmac));
	}

}

int key_dump_network(char* ip_address, unsigned short port, char* output_file) {
	GcKEYS keys;
	
	int got_keys = extract_gc_keys(&keys);
	if(got_keys < 0) return got_keys;
	
	SceUID fd = begin_file_send(ip_address, port, output_file, sizeof(GcKEYS));
	PRINT_STR("fd = %x\n", fd);
	if(fd < 0) return fd;
	
	int wr = file_send_data(fd, &keys, sizeof(GcKEYS));
	PRINT_STR("wr = %x (sizeof = %x)\n", wr, sizeof(GcKEYS));
	end_file_send(fd);

	if(wr == 0) return -1;
	if(wr < 0) return wr;
	if(wr != sizeof(GcKEYS)) return (wr * -1);

	return 0;
}

int key_dump(char* output_file) {
	GcKEYS keys;
	
	int got_keys = extract_gc_keys(&keys);
	if(got_keys < 0) return got_keys;
	
	SceUID fd = sceIoOpen(output_file, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	PRINT_STR("fd = %x\n", fd);
	if(fd < 0) return fd;
	
	int wr = sceIoWrite(fd, &keys, sizeof(GcKEYS));
	PRINT_STR("wr = %x (sizeof = %x)\n", wr, sizeof(GcKEYS));
	sceIoClose(fd);

	if(wr == 0) return -1;
	if(wr < 0) return wr;
	if(wr != sizeof(GcKEYS)) return (wr * -1);

	return 0;
}


void decrypt_packet18_klic(uint8_t* secondaryKey0, uint8_t* packet18, uint8_t* klic) {
	char wbuf[0x30];
	
	AES_CBC_decrypt(packet18+3, wbuf, sizeof(wbuf), secondaryKey0, 0x10, ZERO_IV);
	
	memcpy(klic, wbuf+0x10, 0x20);
}

void decrypt_packet20_rifbuf(uint8_t* secondaryKey0, uint8_t* packet20, uint8_t* rifBuffer) {
	char wbuf[0x40];
	
	AES_CBC_decrypt(packet20+3, wbuf, sizeof(wbuf), secondaryKey0, 0x10, ZERO_IV);

	memcpy(rifBuffer, wbuf+0x18, 0x20);

}

void wait_for_gc_auth() {
	int res = ResetCmd20Input();
	PRINT_STR("ResetCmd20Input = %x\n", res);

	// check if there is already a GC inserted, if there is 
	// reset the gc device to capture authentication step
	// we, dont do this if there is not a gc inserted, incase someone is using an sd2vita.
	if( file_exist("gro0:") || file_exist("grw0:") || device_exist(BLOCK_DEVICE_MEDIAID) ) {
		res = ResetGc();
		PRINT_STR("ResetGc = %x\n", res);			
	}

	while(!HasCmd20Captured()) { sceKernelDelayThread(1000); };
}

uint8_t verify_klic_keys(GcKEYS* keys) {
	char got_final_keys[SHA256_BLOCK_SIZE];
	char expected_final_keys[SHA256_BLOCK_SIZE];
	int res = GetFinalKeys(got_final_keys);
	if(res < 0) goto error;
	
	// final key should == sha256 of klic key
	SHA256_CTX ctx;
	
	sha256_init(&ctx);
	sha256_update(&ctx, keys, sizeof(GcKEYS));
	sha256_final(&ctx, expected_final_keys);
	
	if(memcmp(got_final_keys, expected_final_keys, SHA256_BLOCK_SIZE) == 0) {
		return 1;
	}
error:	
	PRINT_STR("verify_klic_keys failed !\n");
	PRINT_STR("got_final_keys ");
	PRINT_BUFFER(got_final_keys);
	
	PRINT_STR("expected_final_keys ");
	PRINT_BUFFER(expected_final_keys);
	
	return 0;
}

uint8_t verify_rif_keys(GcKEYS* keys) {
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
	PRINT_STR("folder = %s\n", folder);
	
	char TITLE_ID[12];
	int res = read_first_filename(folder, TITLE_ID, sizeof(TITLE_ID));
	PRINT_STR("read_first_filename license folder res = 0x%x\n",res);
	if(res < 0) ERROR(-1);

	PRINT_STR("TITLE_ID = %s\n", TITLE_ID);
	snprintf(folder, MAX_PATH, "gro0:/license/app/%s", TITLE_ID);
	PRINT_STR("folder = %s\n", folder);
	
	// get rif name from license/titleid folder
	char RIF_NAME[0x50];
	res = read_first_filename(folder, RIF_NAME, sizeof(RIF_NAME));
	PRINT_STR("read_first_filename license titleid folder res = 0x%x\n",res);
	if(res < 0) ERROR(-2);
	
	PRINT_STR("RIF_NAME = %s\n", RIF_NAME);
	snprintf(folder, MAX_PATH, "gro0:/license/app/%s/%s", TITLE_ID, RIF_NAME);
	
	PRINT_STR("folder = %s\n", folder);
	
	// read the hash from the rif file ..
	
	SceUID fd = sceIoOpen(folder, SCE_O_RDONLY, 0777);
	PRINT_STR("rif fd = %x\n", fd);
	if(fd < 0) ERROR(-3);
	
	uint64_t loc = sceIoLseek(fd, 0xE0, SCE_SEEK_SET);
	PRINT_STR("sceIoLseek loc = %llx\n", loc);
	if(loc != 0xE0) ERROR(-4);
	
	res = sceIoRead(fd, got_final_rif_hash, SHA1_BLOCK_SIZE);
	PRINT_STR("sceIoRead res = %x\n", res);
	if(res != SHA1_BLOCK_SIZE) ERROR(-5);
	
	sceIoClose(fd);
	
	// final rif hash should == 0xe0 in rif file
	
	if(memcmp(expected_final_rif_hash, got_final_rif_hash, SHA1_BLOCK_SIZE) == 0){
		return 1;
	}

	PRINT_STR("verify_rif_keys failed!\n");
	
	return 0;
error:	
	if(fd > 0)
		sceIoClose(fd);
	return 0;
	
}

int extract_gc_keys(GcKEYS* keys) {
	if(HasCmd20Captured()) {
		// get captured cmd56 authentication data
		CommsData cmdData;
		GetLastCmd20Input(&cmdData);
		int keyId = GetLastCmd20KeyId();
		PRINT_STR("keyId = %x\n", keyId);
		
		
		uint8_t masterKey[0x10];
		derive_master_key(cmdData.packet6, masterKey, keyId);

		PRINT_STR("masterKey ");
		PRINT_BUFFER(masterKey);

		uint8_t secondaryKey0[0x10];
		AES_CBC_decrypt(cmdData.packet9, secondaryKey0, sizeof(secondaryKey0), masterKey, sizeof(masterKey), ZERO_IV);

		PRINT_STR("secondaryKey0 ");
		PRINT_BUFFER(secondaryKey0);
	
			
		// decrypt klic part from packet18
		decrypt_packet18_klic(secondaryKey0, cmdData.packet18, keys->klic);

		// decrypt rif part from packet20
		decrypt_packet20_rifbuf(secondaryKey0, cmdData.packet20, keys->rifbuf);
		
		// verify klic key
		if(!verify_klic_keys(keys)) return -3;
		
		// verify rif buffer
		if(file_exist("gro0:"))
			if(!verify_rif_keys(keys)) return -2;

		
		return 0;
	}
	return -1;
}