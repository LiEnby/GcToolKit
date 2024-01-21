/* 
	psp2spl usage example by SKGleba
	This software may be modified and distributed under the terms of the MIT license.
	See the LICENSE file for details.
*/
#include <stdio.h>
#include <stdarg.h>
#include <vitasdkkern.h>
#include <taihen.h>
#include "payload/payload.h"

static char AUTH_INFO[] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xc0, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x09, 0x80, 0x03, 0x00, 0x00, 0xc3, 0x00, 0x00, 0x00, 0x80, 0x09, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static void* comms_va = NULL;


typedef struct bridge_s {
	uint32_t paddr;
	uint32_t args[8]; // 4 regs + 0x10 stack
} bridge_s;

int fud_bridge_exec(uint32_t exec_addr, uint32_t argc, uint32_t *argv) {
	if (!comms_va)
		return -1;
	if (!argv || !exec_addr)
		return -2;

	bridge_s *bridge = comms_va;
	memset(bridge, 0, sizeof(bridge_s));
	bridge->paddr = exec_addr;
	for (int i = 0; i < argc; ++i)
		bridge->args[i] = argv[i];

	return spl_exec_code(payload_nmp, payload_nmp_len, 0x1C000000, 1);
}

SceSblSmCommId sm_start(){
	SceAuthInfo ctx;
	memset(&ctx,0,sizeof(SceAuthInfo));
	memcpy(&ctx.request,&AUTH_INFO,0x90);
	ctx.media_type = 2;
	ctx.self_type = 2;

	SceSblSmCommId f00d_id = 0;
	int res = ksceSblSmCommStartSmFromFile(0, "os0:/sm/gcauthmgr_sm.self", 0, &ctx, &f00d_id);	
	if(res == 0)
		return f00d_id;
	return -1;
}

int sm_end(SceSblSmCommId id){
	SceSblSmCommPair endResult;
	int res = ksceSblSmCommStopSm(id, &endResult);
	return res;
}

int alloc_shared_mem() {
	// setup shared memory
	comms_va = NULL;
	SceKernelAllocMemBlockKernelOpt optp;
	optp.size = 0x58;
	optp.attr = 2;
    optp.paddr = 0x1C000000;
    int uid = ksceKernelAllocMemBlock("ssram_cam", 0x10208006, 0x00200000, &optp);
	ksceKernelGetMemBlockBase(uid, (void**)&comms_va);
	memset(comms_va, 0, 0x200000);	
	return uid;
}

void free_shared_mem(int uid) {
	ksceKernelFreeMemBlock(uid);	
}

void decrypt_secondary_key0(void *data, uint32_t key_id,void *packet9,void *out) {
	int uid = alloc_shared_mem();

	// start authmgr_sm
	SceSblSmCommId sm = sm_start();
	ksceDebugPrintf("sm = 0x%08X\n", sm);

	// call decrypt_secondary_key0
	void* output = (comms_va + 0x00100000);
	uint32_t start_location = 0x1C100000;
	
	ksceDebugPrintf("copy data\n");
	memcpy(output, data, 0x20);
	memcpy(output+0x20, packet9, 0x30);
	memset(output+0x20+0x30, 0xFF, 0x10);

	uint32_t decrypt_secondary_key0_args[4];
	decrypt_secondary_key0_args[0] = start_location; // data
	decrypt_secondary_key0_args[1] = key_id; // key_id
	decrypt_secondary_key0_args[2] = start_location + 0x20; // packet9
	decrypt_secondary_key0_args[3] = start_location + 0x20 + 0x30; // out
	
	ksceDebugPrintf("run f00d code\n");
	int ret = fud_bridge_exec(0x4a000 + 0x1CB6, 4, decrypt_secondary_key0_args); // decrypt_secondary_key0
	
	ksceDebugPrintf("fud_bridge_exec = 0x%08X\n", ret);
	
	// copy secondary_key0 to outbuffer
	memcpy(out, output + 0x20 + 0x30, 0x10);
	
	free_shared_mem(uid);
	sm_end(sm);
	
}